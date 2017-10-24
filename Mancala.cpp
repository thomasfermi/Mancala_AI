#include <vector>
#include <iostream>
#include <algorithm>
#include <assert.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

class Mancala { //implements a Mancala board game. Based on the rules given on https://www.hackerrank.com/challenges/mancala6
private:
    vector<vector<int> > holes;
    //holes[i] will be an array with 6 numbers, each representing the number of marbles in the hole of player i, (i=0,1)
    vector<int> mancalas;
    //mancalas[i] represents how many marbles are in mancala of player i, (i=0,1)
public:
    //default constructor
    Mancala()
    { 
        for (int pid = 0; pid < 2; pid++) {
            holes.push_back(vector<int>{ 4, 4, 4, 4, 4, 4 });
            mancalas.push_back(0);
        }
    }

    //output function for printing the board
    friend ostream& operator<<(ostream& os, const Mancala& M)
    {
        //top board
        os << "##########################################" << endl;
        os << ">>> ";
        os << M.mancalas[1] << " | ";
        for (int i = 5; i >= 0; i--) {
            os << M.holes[1][i] << " ";
            if (M.holes[1][i] < 10)
                os << " ";
        }
        os << "| " << endl;
        os << ">>>   | ";
	if (M.mancalas[1] >= 10) //do this to align the "|" symbol
            os << " ";

        //bottom board
        for (int i = 0; i < 6; i++) {
            os << M.holes[0][i] << " ";
            if (M.holes[0][i] < 10)
                os << " ";
        }
        os << "| " << M.mancalas[0] << endl;
        os << "##########################################" << endl;
        return os;
    }

    //performs move of player and returns true if extra move is allowed
    bool make_move(int player, int hole)
    {
        assert(player == 1 or player == 2);
        assert(hole < 6);
        int pid = player - 1; //nicer for zero based indexing
        int n_marbles = holes[pid][hole];
        holes[pid][hole] = 0; //take out the marbles
        assert(n_marbles > 0);
        while (n_marbles > 0) {
            hole++; //go one hole further (counter-clockwise)
            hole = hole % 14; //use periodicity of board for the rare case that we have 14 or more marbles
            if (add_to_hole(pid, hole)) //try to add marble.
                n_marbles--; //if marble was added, we have one less
        }
        return finish_on_hole(pid, hole);
    }

    //add to specified hole. if that hole is opponents mancala, return false (we do not add to opponents mancala)
    bool add_to_hole(int pid, int hole)
    {
        if (hole < 6)
            holes[pid][hole]++;
        else if (hole == 6)
            mancalas[pid]++;
        else if (hole < 13) //adding to hole of opponent (who has player id !pid)
            holes[!pid][hole - 7]++;
        else //the hole is the opponents mancala into which we do not put marbles
            return false;
        return true;
    }

    //apply rules for finishing on certain spots. return true if player gets extra move
    bool finish_on_hole(int pid, int hole)
    {
        if (hole < 6 and holes[pid][hole] == 1) { //dropped last marble into empty hole on your side
            int opposite_hole = 5 - hole;
            int n_transfer_marbles = holes[!pid][opposite_hole];
            holes[!pid][opposite_hole] = 0;
            holes[pid][hole] += n_transfer_marbles;
        }
        else if (hole == 6) //if it is your mancala
            return true;
        return false;
    }

    //returns true if the move is legal. Gives user some feedback via cout otherwise
    bool is_legal_move(int player, int hole)
    {
        assert(player == 1 or player == 2);
        if (hole < 0 or hole > 5) {
            cout << "Illegal Move. Enter 0,1,2,3,4 or 5." << endl;
            return false;
        }
        if (holes[player - 1][hole] == 0) {
            cout << "There are no marbles in hole " << hole << ". Choose a hole with marbles in it!" << endl;
            return false;
        }
        return true;
    }

    //if gamestate is final, clear up the board (winner marbles into winner mancala) and return true, else: do nothing
    bool final_game_state()
    {
        bool p1_empty = true;
        bool p2_empty = true;
        for (int i = 0; i < 6; i++) {
            p1_empty = p1_empty and (holes[0][i] == 0);
            p2_empty = p2_empty and (holes[1][i] == 0);
        }
        if (p1_empty or p2_empty) {
            //we will clean up the board of player with id=clear_index
            int clear_index = p1_empty; //if p1 empty, clear_index=1 (p2), else p2 is empty and clear_index=0 (p1)
            int mancala_add = 0;
            for (int i = 0; i < 6; i++) {
                mancala_add += holes[clear_index][i];
                holes[clear_index][i] = 0;
            }
            mancalas[clear_index] += mancala_add;
            return true;
        }
        return false;
    }

    //print the winner to cout. Note that we do not include this printing in final_game_state(), since final_game_state() is called
    //several times by the AI when it goes down the min-max-tree
    void print_winner()
    {
        cout << "You have " << mancalas[0] << " marbles. The AI has " << mancalas[1] << " marbles." << endl;
        if (mancalas[0] > mancalas[1])
            cout << "You won." << endl;
        if (mancalas[0] == mancalas[1])
            cout << "It's a draw." << endl;
        else
            cout << "You lost." << endl;
    }

    // return a list of pairs. Each pair consists of a game state s' and an action a, whereby s' is a successor of the current game state s under action a.
    // Note that the action is a vector<int>, since an action can consist of picking several holes (extra move rule)
    vector<pair<Mancala, vector<int> > >
    find_all_children_and_actions(int pid, vector<int> actions_so_far = {})
    { //find all game states (children) that player pid can reach
        vector<pair<Mancala, vector<int> > > children_and_actions;
        for (int nexthole = 0; nexthole < 6; nexthole++) { //for each hole
            if (holes[pid][nexthole] > 0) { //that is not empty
                Mancala nextM = *this; //make a copy of this object and save it in nextM
                bool extra_move = nextM.make_move(pid + 1, nexthole); //nextM changes due to action "nexthole"
                pair<Mancala, vector<int> > child_and_actions;
                child_and_actions.first = nextM;
                vector<int> actions = actions_so_far;
                actions.push_back(nexthole);
                child_and_actions.second = actions;
                if (not extra_move)
                    children_and_actions.push_back(child_and_actions);
                else {
                    //recursive call if there is an extra move, since we can have additional actions
                    auto further_children_and_actions = nextM.find_all_children_and_actions(pid, actions);
                    children_and_actions.insert(children_and_actions.end(), further_children_and_actions.begin(),
                        further_children_and_actions.end());
                }
            }
        }
        return children_and_actions;
    }

    //perform iterative deepening and stop after max_duration. Return an int corresponding to the action the AI came up with
    int AI_iterative_deepening_move_decision(int player, int max_duration_in_milliseconds)
    {
        int pid = player - 1; //for zero based indexing

        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(t2 - t1).count();

        int best_hole = -1;
        int max_depth = 4; //depth 4 should always be possible on a current computer. Hence start with that as minimum depth

        while (duration < max_duration_in_milliseconds) {
            best_hole = AI_move_decision(player, max_depth);
            t2 = high_resolution_clock::now();
            duration = duration_cast<milliseconds>(t2 - t1).count();
            max_depth++;
        }
        return best_hole;
    }

    // decide for the best possible move when going max_depth into the game tree. Use heuristic at depth==max_depth
    int AI_move_decision(int player, int max_depth)
    {
        int pid = player - 1; //for zero based indexing
        int best_hole = -1;

        if (pid == 0) {
            int best_value = -1000000;

            auto children_and_actions = find_all_children_and_actions(pid);
            for (auto child_and_actions : children_and_actions) {
                int value = alpha_beta(child_and_actions.first, max_depth, -1000000, 1000000,
                    !pid); //the value is given by: optimal play from pid onward
                if (value > best_value) {
                    best_value = value;
                    best_hole = child_and_actions.second[0];
                }
            }
        }
        else {
            int min_value = 1000000;

            auto children_and_actions = find_all_children_and_actions(pid);
            for (auto child_and_actions : children_and_actions) {
                int value = alpha_beta(child_and_actions.first, max_depth, -1000000, 1000000,
                    !pid); //the value is given by: optimal play from pid onward
                if (value < min_value) {
                    min_value = value;
                    best_hole = child_and_actions.second[0];
                }
            }
        }
        return best_hole;
    }

    // a very simple heuristic
    int heuristic()
    { //convention: player 1 (pid=0) is maximizing player, player 2 is minimizing player
        int h = mancalas[0] - mancalas[1];
        for (int i = 0; i < 6; i++) {
            h += (holes[0][i] - holes[1][i]);
        }
        return h;
    }

    // alpha beta pruning
    int alpha_beta(Mancala& M, int depth, int alpha, int beta, int pid)
    { //returns value v of a Mancala game state
        if (depth == 0 or M.final_game_state()) {
            return M.heuristic();
        }
        if (pid == 0) { //if maximizing player
            int v = -1000000;
            for (auto child_and_actions : M.find_all_children_and_actions(pid)) {
                v = max(v, alpha_beta(child_and_actions.first, depth - 1, alpha, beta, 1));
                alpha = max(alpha, v);
                if (beta <= alpha)
                    break; //beta cut off
            }
            return v;
        }
        else {
            int v = 1000000;
            for (auto child_and_actions : M.find_all_children_and_actions(pid)) {
                v = min(v, alpha_beta(child_and_actions.first, depth - 1, alpha, beta, 0));
                beta = min(beta, v);
                if (beta <= alpha)
                    break; //beta cut off
            }
            return v;
        }
    }

}; // end class Mancala

int main(void)
{
    Mancala MGame = Mancala();
    int player_move, AI_move;

    cout << "Welcome to Mancala. Please choose an AI strength of 1 (weak), 2 (normal), or 3 (hard)" << endl
         << ">>> ";
    int thinking_time;
    bool valid_AI_choice = false;
    while (not valid_AI_choice) {
        cin >> thinking_time;
        valid_AI_choice = (thinking_time == 1 or thinking_time == 2 or thinking_time == 3);
        if (not valid_AI_choice)
            cout << "Invalid choice. Please enter 1,2, or 3" << endl
                 << " >>> ";
    }
    thinking_time *= 400;
    cout << "Thanks. You are the player at the bottom of the board. Here is the board:" << endl;
    cout << MGame << endl;

    while (true) {
        cout << "Which hole do you pick (1-6)?" << endl;
        bool move_allowed = false;
        while (not move_allowed) {
            cout << ">>> ";
            cin >> player_move;
            player_move -= 1; //zero based indexing
            move_allowed = MGame.is_legal_move(1, player_move);
        }
        MGame.make_move(1, player_move);
        cout << "Result of your move:" << endl;
        cout << MGame << endl;
        if (MGame.final_game_state()) {
            MGame.print_winner();
            break;
        }
        cout << "AI is making a move, resulting in ..." << endl;
        AI_move = MGame.AI_iterative_deepening_move_decision(2, thinking_time);
        MGame.make_move(2, AI_move);
        cout << MGame << endl;
        if (MGame.final_game_state()) {
            MGame.print_winner();
            break;
        }
    }
    return 0;
}
