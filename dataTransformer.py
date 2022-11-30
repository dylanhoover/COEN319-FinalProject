#Program to read combined_data_1.txt and order the lines for each movie
import sys
import itertools

def main():
    file1 = open('combined_data.txt', 'r')
    output = open('combined_data_ordered.txt', 'w+')

    colon_lines = []
    for i, line in enumerate(file1):
        if line.strip().endswith(':'):
            colon_lines.append(i)
    
    file1.close()
    file1 = open('combined_data.txt', 'r')
    fullLines = file1.readlines()

    for i in range(len(colon_lines)):
        if i == len(colon_lines) - 1:
            lines = fullLines[colon_lines[i]+1:]
        else:
            lines = fullLines[colon_lines[i]+1:colon_lines[i+1]]

        # lines = [line.strip() for line in lines]
        lines.sort(key=lambda x: int(x.split(',')[0]))
        output.write(str(i+1)+":\n")
        for line in lines:
            output.write(line)
    
    file1.close()
    output.close()

if __name__ == "__main__":
    main()