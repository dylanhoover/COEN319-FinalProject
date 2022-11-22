#program to combine 4 text files into one

import sys

def main():
    #open files
    file1 = open('combined_data_1.txt', 'r')
    file2 = open('combined_data_2.txt', 'r')
    file3 = open('combined_data_3.txt', 'r')
    file4 = open('combined_data_4.txt', 'r')
    #open output file
    output = open('combined_data.txt', 'w')
    #read files
    file1_lines = file1.readlines()
    file2_lines = file2.readlines()
    file3_lines = file3.readlines()
    file4_lines = file4.readlines()
    #combine files
    combined = file1_lines + file2_lines + file3_lines + file4_lines
    #write to output file
    for line in combined:
        output.write(line)
    #close files
    file1.close()
    file2.close()
    file3.close()
    file4.close()
    output.close()

if __name__ == '__main__':
    main()