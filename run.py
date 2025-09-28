import os
import argparse

def process_gitignore(file_path):
    """Process a single .gitignore file, deleting lines containing 'cmake' or 'config.h'"""
    try:
        # Read file content
        with open(file_path, 'r', encoding='utf-8') as f:
            lines = f.readlines()
        
        # Filter out lines containing 'cmake/' or 'config.h' (case insensitive)
        new_lines = [
            line for line in lines 
            if 'cmake/' not in line.lower() 
            and 'config.h' not in line.lower()
        ]
        
        # Write if there is any change in content
        if len(new_lines) != len(lines):
            with open(file_path, 'w', encoding='utf-8') as f:
                f.writelines(new_lines)
            print(f"Processed: {file_path}")
            return True
        return False
        
    except Exception as e:
        print(f"An error occurred while processing file {file_path}: {str(e)}")
        return False

def find_and_process_gitignores(root_dir):
    """Recursively find and process all .gitignore files"""
    if not os.path.isdir(root_dir):
        print(f"Error: {root_dir} is not a valid directory")
        return
    
    processed_count = 0
    
    # Recursively traverse directory
    for dirpath, _, filenames in os.walk(root_dir):
        if '.gitignore' in filenames:
            gitignore_path = os.path.join(dirpath, '.gitignore')
            if process_gitignore(gitignore_path):
                processed_count += 1
    
    print(f"\nThe processing is completed, and a total of {processed_count} .gitignore files have been modified")

if __name__ == "__main__":
    # Set command line parameters
    parser = argparse.ArgumentParser(description='递归处理文件夹中的所有.gitignore文件，删除包含cmake或config.h的行')
    parser.add_argument('directory', help='要处理的根目录路径')
    
    args = parser.parse_args()
    find_and_process_gitignores(args.directory)
