#pragma once

// izzint - TODO: these other exceptions should really be implemented

class EOFException : public std::runtime_error
{

};

class IllegalArgumentException : public std::runtime_error
{
public:
	std::wstring information;

	IllegalArgumentException(const std::wstring& information);
};

class IOException : public std::runtime_error
{
public:
	std::wstring information;

	IOException(const std::wstring& information);
};

class RuntimeException : public std::runtime_error
{
public:
	RuntimeException(const std::wstring& information);
};