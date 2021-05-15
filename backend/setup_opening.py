from os import truncate
import re

import json

file = open('extracted.pgn', 'r')#

move_dict = {}
for line in file: 
    #print("Hallo")
    moves = line.strip().split(" ")[:20]
    current_dict_level = move_dict
    for move in moves:
        if not move in current_dict_level:
            current_dict_level[move] = {}

        current_dict_level = current_dict_level[move]
    #print(move_dict)
    #print("Hallo")

print("write")
with open('my_openings.json', 'w') as outfile:
    json.dump(move_dict, outfile)