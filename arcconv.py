import sys, struct
from zipfile import ZipFile, Path
from io import BytesIO


def usage(exe_name: str):
    print(f"usage: {exe_name} <command> <infile> <outfile>")
    print("  command: either arc2zip or zip2arc")
    sys.exit(1)


def main():
    exe_name = sys.argv[0]
    if len(sys.argv) != 4: usage(exe_name)
    command = sys.argv[1]
    in_filename = sys.argv[2]
    out_filename = sys.argv[3]

    if command == "arc2zip":
        arc_to_zip(in_filename, out_filename)
    elif command == "zip2arc":
        zip_to_arc(in_filename, out_filename)
    else:
        usage(exe_name)


def arc_to_zip(in_filename: str, out_filename: str):
    with open(in_filename, "rb") as file:
        data = file.read()
        buf = BytesIO(data)

    with ZipFile(out_filename, "w") as zipfile:
        entry_count = struct.unpack(">I", buf.read(4))[0]
        files = []

        for _ in range(entry_count):
            strlen = struct.unpack(">H", buf.read(2))[0]
            fmt = f">{strlen}s2I"
            fmt_size = struct.calcsize(fmt)
            (filename, offset, size) = struct.unpack(fmt, buf.read(fmt_size))
            filename = filename.decode("UTF-8").strip("\0").replace("\\", "/")
            file_path = Path(zipfile, filename)
            files.append((filename, data[offset:offset + size]))

#        files.sort(key=lambda pair: pair[0])
        for filename, contents in files:
            zipfile.writestr(filename, contents)


def zip_to_arc(in_filename: str, out_filename: str):
    data_buf = bytearray()

    with open(out_filename, "wb") as out_file:
        files = []

        with ZipFile(in_filename, "r") as zip_file:
            files.extend((file.filename, zip_file.read(file)) for file in zip_file.filelist)

#        files.sort(key=lambda pair: pair[0])

        header_length = (
            sum(len(file[0].encode('utf-8')) for file in files)
            + len(files) * struct.calcsize(f">H2I")
            + struct.calcsize("I")
        )

        out_file.write(struct.pack(">I", len(files)))

        for filename, contents in files:
            offset = header_length + len(data_buf)
            file_path = filename.replace("/", "\\").encode('utf-8')
            out_file.write(struct.pack(
                f">H{len(file_path)}s2I", len(file_path), file_path, offset, len(contents)
            ))
            data_buf.extend(contents)

        out_file.write(data_buf)

if __name__ == "__main__":
    main()

