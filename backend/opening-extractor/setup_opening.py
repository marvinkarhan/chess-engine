from os import truncate
import re
import sys, getopt
import json

def extract_moves_from_pgn_to_dict(filename: str, amount_of_moves: int) -> dict:
    move_dict = {}
    file = open(filename, 'r')
    for line in file: 
        all_moves = line.strip().split(" ")
        all_moves_len = len(all_moves)
        if(amount_of_moves >= all_moves_len):
            moves = all_moves[:all_moves_len - 1]
        else:
            moves = all_moves[:amount_of_moves]
        current_dict_level = move_dict
        for move in moves:
            if not move in current_dict_level:
                current_dict_level[move] = {}
            current_dict_level = current_dict_level[move]

    return move_dict

def write_dict_to_json(dict: dict, filename: str) -> None:
    with open(filename, 'w') as outfile:
        json.dump(dict, outfile)


if __name__ == '__main__':

    arguments = sys.argv[1:]
    options = "i:o:m:"
    long_options = ["input =", "output =", "moves ="]

    input_file, output_file, moves = None, None, None
    try:
        arguments, values = getopt.getopt(arguments, options, long_options)
        for argument, argument_value in arguments:
            if argument in ("-i", "--input"):
                input_file = argument_value
            elif argument in ("-m", "--moves"):
                moves = int(argument_value)
            elif argument in ("-o", "--output"):
                output_file = argument_value
                
    except getopt.error as err:
        # output error, and return with an error code
        print (str(err))
    
    if not input_file or not output_file:
        print("ERROR: No input file or output file given! Use -i for input file, -o for ouput file and -m for moves")
        exit()
    if not moves:
        print("No moves given: Defaults to 4")
        moves = 4

    print("START MOVE EXTRACTION")
    moves_dict = extract_moves_from_pgn_to_dict(input_file, moves)
    print("WRITE MOVES TO JSON")
    write_dict_to_json(moves_dict, output_file)
    print("FINISHED WRITING MOVES")
