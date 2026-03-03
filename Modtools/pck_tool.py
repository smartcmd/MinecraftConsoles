import os
import re
import tkinter as tk
from tkinter import filedialog, messagebox
import customtkinter as ctk
from tkinterdnd2 import DND_FILES, TkinterDnD

PNG_HEADER = b"\x89PNG\r\n\x1a\n"

LANGUAGES = {
    "English": {
        "title": "PCK Skin Tool",
        "drop": "DRAG & DROP PCK FILES HERE",
        "extract_folder": "Extract Folder",
        "create_pck": "Create PCK",
        "language": "Language",
        "theme": "Theme:",
        "logs": "Activity Log",
        "success_extract": "Extraction completed",
        "success_create": "PCK created successfully",
        "error_no_png": "No PNG files found",
        "error_no_png_in_pck": "No PNG inside PCK"
    },
    "Español": {
        "title": "Herramienta PCK",
        "drop": "ARRASTRA ARCHIVOS .PCK AQUÍ",
        "extract_folder": "Extraer Carpeta",
        "create_pck": "Crear PCK",
        "language": "Idioma",
        "theme": "Tema:",
        "logs": "Registro de Actividad",
        "success_extract": "Extracción completada",
        "success_create": "PCK creado correctamente",
        "error_no_png": "No se encontraron PNG",
        "error_no_png_in_pck": "Sin PNG dentro del PCK"
    }
}

ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")

class PCKTool(TkinterDnD.Tk):
    def __init__(self):
        super().__init__()
        

        self.language = "English"
        self.translations = LANGUAGES[self.language]

        current_mode = 1 if ctk.get_appearance_mode() == "Dark" else 0
        bg_color = ctk.ThemeManager.theme["CTk"]["fg_color"][current_mode]
        self.config(background=bg_color)

        self.title(self.translations["title"])
        self.geometry("850x570")
        
        # Grid layout
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(0, weight=1)

        # --- SIDEBAR ---
        self.sidebar = ctk.CTkFrame(
            self, 
            width=220, 
            corner_radius=0,
            fg_color=("#EDEDED", "#1A1A1A")
        )
        self.sidebar.grid(row=0, column=0, sticky="nsew")
        self.sidebar.grid_rowconfigure(6, weight=1)

        self.logo_label = ctk.CTkLabel(
            self.sidebar, 
            text="PCK EXTRACTOR", 
            font=ctk.CTkFont(size=20, weight="bold"),
            anchor="w"
        )
        self.logo_label.grid(row=0, column=0, padx=20, pady=(20, 30), sticky="w")

        self.lang_label = ctk.CTkLabel(self.sidebar, text=self.translations["language"], anchor="w")
        self.lang_label.grid(row=1, column=0, padx=20, pady=(10, 0), sticky="w")
        
        self.lang_menu = ctk.CTkOptionMenu(self.sidebar, values=list(LANGUAGES.keys()), command=self.change_language)
        self.lang_menu.set(self.language)
        self.lang_menu.grid(row=2, column=0, padx=20, pady=10, sticky="w")

    #    self.theme_label = ctk.CTkLabel(self.sidebar, text=self.translations["theme"], anchor="w")
    #    self.theme_label.grid(row=3, column=0, padx=20, pady=(20, 0), sticky="w")
        
    #    self.appearance_mode_menu = ctk.CTkOptionMenu(self.sidebar, values=["Dark", "Light"], command=self.change_appearance_mode)
    #    self.appearance_mode_menu.grid(row=4, column=0, padx=20, pady=10, sticky="w")

        # --- MAIN CONTENT ---
        self.main_container = ctk.CTkFrame(self, fg_color="transparent")
        self.main_container.grid(row=0, column=1, padx=20, pady=20, sticky="nsew")
        self.main_container.grid_columnconfigure(0, weight=1)
        self.main_container.grid_rowconfigure(3, weight=1)

        # Drop Zone
        self.drop_frame = ctk.CTkFrame(
            self.main_container, 
            border_width=2, 
            border_color="#3B8ED0",
            fg_color=("white", "#2B2B2B")
        )
        self.drop_frame.grid(row=0, column=0, sticky="nsew", pady=(0, 20), ipady=50)
        self.drop_frame.grid_columnconfigure(0, weight=1)
        self.drop_frame.grid_rowconfigure(0, weight=1)

        self.drop_label = ctk.CTkLabel(self.drop_frame, text=self.translations["drop"], 
                                       font=ctk.CTkFont(size=16, weight="bold"))
        self.drop_label.grid(row=0, column=0)

        self.drop_frame.drop_target_register(DND_FILES)
        self.drop_frame.dnd_bind("<<Drop>>", self.drop_file)

        # Action Buttons
        self.button_frame = ctk.CTkFrame(self.main_container, fg_color="transparent")
        self.button_frame.grid(row=1, column=0, sticky="ew")
        self.button_frame.grid_columnconfigure((0, 1), weight=1)

        self.extract_button = ctk.CTkButton(self.button_frame, text=self.translations["extract_folder"], 
                                           height=45, fg_color="#1f538d", command=self.extract_folder)
        self.extract_button.grid(row=0, column=0, padx=5, pady=10, sticky="ew")

        self.create_button = ctk.CTkButton(self.button_frame, text=self.translations["create_pck"], 
                                          height=45, fg_color="#2b8a3e", hover_color="#1e5f2b", 
                                          command=self.create_pck)
        self.create_button.grid(row=0, column=1, padx=5, pady=10, sticky="ew")

        # Logs
        self.log_label = ctk.CTkLabel(self.main_container, text=self.translations["logs"], font=ctk.CTkFont(weight="bold"))
        self.log_label.grid(row=2, column=0, sticky="w", pady=(10, 0))
        
        self.textbox = ctk.CTkTextbox(self.main_container, corner_radius=10, border_width=1)
        self.textbox.grid(row=3, column=0, sticky="nsew", pady=(5, 0))
        
        self.log("System initialized.")

    def log(self, message):
        self.textbox.insert("end", f"[*] {message}\n")
        self.textbox.see("end")

    def change_appearance_mode(self, new_mode: str):
        ctk.set_appearance_mode(new_mode)
        current_mode = 1 if new_mode == "Dark" else 0
        bg_color = ctk.ThemeManager.theme["CTk"]["fg_color"][current_mode]
        self.config(background=bg_color)

    def change_language(self, choice):
        self.language = choice
        self.translations = LANGUAGES[self.language]
        self.refresh_ui()

    def refresh_ui(self):
        self.title(self.translations["title"])
        self.lang_label.configure(text=self.translations["language"])
        self.theme_label.configure(text=self.translations["theme"])
        self.drop_label.configure(text=self.translations["drop"])
        self.extract_button.configure(text=self.translations["extract_folder"])
        self.create_button.configure(text=self.translations["create_pck"])
        self.log_label.configure(text=self.translations["logs"])

    def extract_pck(self, filepath):
        try:
            filename = os.path.basename(filepath)
            with open(filepath, "rb") as f:
                data = f.read()
            positions = [m.start() for m in re.finditer(PNG_HEADER, data)]
            if not positions:
                self.log(f"Error: No PNGs in {filename}")
                return
            output_dir = os.path.splitext(filepath)[0] + "_extracted"
            os.makedirs(output_dir, exist_ok=True)
            for i, pos in enumerate(positions):
                next_pos = positions[i+1] if i+1 < len(positions) else len(data)
                chunk = data[pos:next_pos]
                with open(os.path.join(output_dir, f"skin_{i+1:03d}.png"), "wb") as out:
                    out.write(chunk)
            self.log(f"Success: {filename} -> {len(positions)} skins extracted.")
        except Exception as e:
            self.log(f"Critical Error: {str(e)}")

    def drop_file(self, event):
        raw_data = event.data
        if raw_data.startswith('{') and raw_data.endswith('}'):
            paths = [raw_data[1:-1]]
        else:
            paths = self.tk.splitlist(raw_data)
        for path in paths:
            if path.lower().endswith(".pck"):
                self.extract_pck(path)
        messagebox.showinfo("Process", self.translations["success_extract"])

    def extract_folder(self):
        folder = filedialog.askdirectory()
        if not folder: return
        pck_files = [f for f in os.listdir(folder) if f.lower().endswith(".pck")]
        for file in pck_files:
            self.extract_pck(os.path.join(folder, file))
        messagebox.showinfo("OK", self.translations["success_extract"])

    def create_pck(self):
        folder = filedialog.askdirectory()
        if not folder: return
        pngs = sorted([f for f in os.listdir(folder) if f.lower().endswith(".png")])
        if not pngs:
            messagebox.showerror("Error", self.translations["error_no_png"])
            return
        output_path = os.path.join(folder, "custom_skins.pck")
        try:
            with open(output_path, "wb") as out:
                for png in pngs:
                    with open(os.path.join(folder, png), "rb") as f:
                        out.write(f.read())
            self.log(f"Created PCK: {output_path} ({len(pngs)} images)")
            messagebox.showinfo("OK", self.translations["success_create"])
        except Exception as e:
            self.log(f"Error: {str(e)}")

if __name__ == "__main__":
    app = PCKTool()
    app.mainloop()