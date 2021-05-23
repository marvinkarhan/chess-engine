#include "movehelper.h"
#include <vector>
#include <algorithm>
#include <regex>
#include <string>


std::vector<BB> get_lsb_bb_array(BB bb) {
    std::vector<BB> bbs;
    BB new_bb;
    while (bb) {
        int index = pop_lsb(bb);
        new_bb = 1 << index;
        bbs.push_back(new_bb);
    }
    return bbs;
}

std::vector<int> get_lsb_array(BB bb) {
    std::vector<int> indexes;
    while (bb) {
        int index = pop_lsb(bb);
        indexes.push_back(index);
    }
    return indexes;
}


// TODO: Finish
// Move uci_to_Move(std::string uci) {
//     auto regex = std::regex("([a-h][1-8])([a-h][1-8])(.)?"); 
//     auto m = std::regex_match(regex, uci);

//     if (m == null) {
//         print('Invalid uci');
//         return;
//     }

//     promotion = m.group(3);
//     if promotion is not None and promotion not in PROMOTION_OPTIONS_W + PROMOTION_OPTIONS_B:
//         print('Invalid promotion in uci');
//         return;
//     origin_square = ALGEBRAIC_TO_INDEX[m.group(1)];
//     target_square = ALGEBRAIC_TO_INDEX[m.group(2)];
//     return Move(origin_square, target_square, prmtn=promotion);
// }
