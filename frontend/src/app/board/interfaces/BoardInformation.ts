export interface BoardInformation {
    fen?: string;
    moves?: string[];
    evaluation?: number;
    aiMoves?: string[];
    next?: BoardInformation;
    prev?: BoardInformation;
    idx?: number;
}
