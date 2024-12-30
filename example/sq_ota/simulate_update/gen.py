# Python script to convert a binary file to a C-style array in a header file

def generate_header(input_file, output_file):
    try:
        with open(input_file, 'rb') as bin_file:
            binary_data = bin_file.read()
        
        # Create a C array from the binary data
        c_array = ", ".join(f"0x{byte:02X}" for byte in binary_data)

        # Write the header file
        with open(output_file, 'w') as header_file:
            header_file.write("// Auto-generated header file containing binary data\n")
            header_file.write("#ifndef SRC_H\n")
            header_file.write("#define SRC_H\n\n")
            header_file.write(f"const unsigned char src[] = {{\n    {c_array}\n}};\n")
            header_file.write(f"const unsigned int src_len = {len(binary_data)};\n\n")
            header_file.write("#endif // SRC_H\n")

        print(f"Header file '{output_file}' has been generated successfully.")
    except FileNotFoundError:
        print(f"Error: The file '{input_file}' does not exist.")
    except Exception as e:
        print(f"An error occurred: {e}")

# Specify the input binary file and output header file
input_binary_file = "example/sq_ota/Blink.ino.bin"
output_header_file = "src.h"

# Generate the header file
generate_header(input_binary_file, output_header_file)