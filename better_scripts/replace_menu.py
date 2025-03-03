import os
import threading

def process_file(source, newname):
    # Open the original file for reading in binary mode
    with open(source, "rb") as original_file:
        # Define the bytes we want to replace and their replacements
        with open('MenuHEX.find', 'rb') as f1:
                bytes_to_replace = f1.read()
        with open('MenuHEX.rep', 'rb') as f2:
            replacement_bytes = f2.read()
    
        # Read the entire contents of the original file
        file_contents = original_file.read()
    
        # Find the index of the bytes we want to replace
        start_index = file_contents.find(bytes_to_replace)
    
        # Create a copy of the original file
        with open(newname, "wb") as new_file:
            # Write the first part of the original file up to the bytes we want to replace
            new_file.write(file_contents[:start_index])
    
            # Write the replacement bytes
            new_file.write(replacement_bytes)
    
            # Write the last part of the original file from the point where the bytes to replace ends
            new_file.write(file_contents[start_index + len(bytes_to_replace):])


def main():
    # List all .dec files in the directory
    files = [f for f in os.listdir('.') if f.endswith('.dec')]

    # Loop through the files and generate new names
    for filename in files:
        source = os.path.join('.', filename)
        newname = os.path.join('.', 'new', filename)
        thread = threading.Thread(target=process_file, args=(source, newname))
        thread.start()

if __name__ == '__main__':
    main()
