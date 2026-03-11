 #include "FourKitInternal.h"

bool FourKit::Initialize()
{
    String ^ pluginsFolderPath = "plugins";

    try
    {
        if (!Directory::Exists(pluginsFolderPath))
        {
            PluginLogger::LogInfo("fourkit", "Plugins folder does not exist, creating...");
            Directory::CreateDirectory(pluginsFolderPath);
        }

        pluginList->Clear();
        onlinePlayerNames->Clear();

        cli::array<String ^> ^ dllFiles = Directory::GetFiles(pluginsFolderPath, "*.dll");

        PluginLogger::LogInfo("fourkit", String::Format("Found {0} plugin DLL files", dllFiles->Length));

        for each (String ^ dllFile in dllFiles)
        {
            if (!LoadPlugin(dllFile))
            {
                PluginLogger::LogWarn("fourkit", String::Format("Failed to load plugin: {0}", dllFile));
            }
        }

        PluginLogger::LogInfo("fourkit", String::Format("Loaded {0} plugins successfully", pluginList->Count));

        return true;
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error during initialization: {0}", ex->Message));
        return false;
    }
}

void FourKit::Shutdown()
{
    try
    {
        FireEventOnExit();
        pluginList->Clear();
        onlinePlayerNames->Clear();
        PluginLogger::LogInfo("fourkit", "Plugin system shut down");
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error during shutdown: {0}", ex->Message));
    }
}

bool FourKit::LoadPlugin(String ^ pluginPath)
{
    try
    {
        Assembly ^ pluginAssembly = Assembly::LoadFrom(pluginPath);

        if (pluginAssembly == nullptr)
        {
            PluginLogger::LogError("fourkit", String::Format("Failed to load assembly: {0}", pluginPath));
            return false;
        }

        cli::array<Type ^> ^ types = pluginAssembly->GetTypes();

        bool pluginFound = false;

        for each (Type ^ type in types)
        {
            if (type->GetInterface("ServerPlugin") != nullptr && !type->IsInterface)
            {
                try
                {
                    ServerPlugin ^ plugin = safe_cast<ServerPlugin ^>(Activator::CreateInstance(type));

                    if (plugin != nullptr)
                    {
                        pluginList->Add(plugin);
                        PluginLogger::LogInfo("fourkit",
                            String::Format("Loaded plugin: {0} v{1} by {2}",
                                plugin->GetName(), plugin->GetVersion(), plugin->GetAuthor()));
                        pluginFound = true;
                    }
                }
                catch (Exception ^ ex)
                {
                    PluginLogger::LogError("fourkit",
                        String::Format("Failed to instantiate plugin {0}: {1}",
                            type->FullName, ex->Message));
                }
            }
        }

        if (!pluginFound)
        {
            PluginLogger::LogWarn("fourkit",
                String::Format("No ServerPlugin implementations found in assembly: {0}", pluginPath));
        }

        return pluginFound;
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error loading plugin: {0}", ex->Message));
        return false;
    }
}

List<ServerPlugin ^> ^ FourKit::GetLoadedPlugins()
{
    return pluginList;
}

List<Player^>^ FourKit::GetPlayersInDimension(int dimension)
{
	System::Collections::Generic::List<Player^>^ players = gcnew System::Collections::Generic::List<Player^>(0);
	for each (String^ name in onlinePlayerNames)
	{
		Player^ player = FourKitInternal::ResolvePlayerByName(name);
		if (player != nullptr && player->getDimension() == dimension)
		{
			players->Add(player);
		}
	}

	return players;
}

void FourKit::LogPlugin(String ^ pluginName, String ^ message)
{
    PluginLogger::LogPluginInfo(pluginName, message);
}

void FourKit::addListener(Listener ^ listener)
{
    EventManager::RegisterListener(listener);
}

Block ^ FourKit::getBlockAt(double x, double y, double z)
{
    int bx = (int)Math::Floor(x);
    int by = (int)Math::Floor(y);
    int bz = (int)Math::Floor(z);
    int dimension = 0;

    int blockType = NativeBlockCallbacks::GetBlockType(bx, by, bz, dimension);
    int blockData = NativeBlockCallbacks::GetBlockData(bx, by, bz, dimension);

    return gcnew Block(bx, by, bz, dimension, blockType, blockData);
}

Player ^ FourKit::getPlayer(String ^ name)
{
    return FourKitInternal::ResolvePlayerByName(name);
}

World^ FourKit::getWorld(int dimension)
{
	switch (dimension)
	{
	case -1:
	case 0:
	case 1:
		return gcnew World(dimension);
	default:
		return nullptr;
	}
}

World^ FourKit::getWorld(String^ name)
{
	int dimension = 0;
	return FourKitInternal::TryGetDimensionFromWorldName(name, dimension) ? getWorld(dimension) : nullptr;
}

PluginCommand^ FourKit::getCommand(String^ name)
{
	if (String::IsNullOrWhiteSpace(name))
	{
		return nullptr;
	}

	String^ key = name->Trim()->ToLowerInvariant();
	PluginCommand^ command = nullptr;
	if (!commandMap->TryGetValue(key, command))
	{
		command = gcnew PluginCommand(key);
		commandMap->Add(key, command);
	}

	return command;
}

bool FourKit::DispatchPlayerCommand(String^ playerName, String^ commandLine)
{
	String^ label = String::Empty;
	cli::array<String^>^ args = FourKitInternal::ParseCommandArguments(commandLine, label);
	if (String::IsNullOrWhiteSpace(label))
	{
		return false;
	}

	PluginCommand^ command = nullptr;
	if (!commandMap->TryGetValue(label, command) || command == nullptr || command->getExecutor() == nullptr)
	{
		command = nullptr;
		for each (KeyValuePair<String^, PluginCommand^> entry in commandMap)
		{
			PluginCommand^ registeredCommand = entry.Value;
			if (registeredCommand == nullptr || registeredCommand->getExecutor() == nullptr)
			{
				continue;
			}

			if (String::Equals(registeredCommand->getName(), label, StringComparison::OrdinalIgnoreCase))
			{
				command = registeredCommand;
				break;
			}

			List<String^>^ aliases = registeredCommand->getAliases();
			if (aliases == nullptr)
			{
				continue;
			}

			for each (String^ alias in aliases)
			{
				if (!String::IsNullOrWhiteSpace(alias) && String::Equals(alias->Trim(), label, StringComparison::OrdinalIgnoreCase))
				{
					command = registeredCommand;
					break;
				}
			}

			if (command != nullptr)
			{
				break;
			}
		}

		if (command == nullptr)
		{
			return false;
		}
	}

	CommandSender^ sender = FourKitInternal::ResolvePlayerByName(playerName);
	if (sender == nullptr)
	{
		sender = gcnew Player(playerName);
	}

	return command->execute(sender, label, args);
}
