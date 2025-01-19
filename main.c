#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLKSIZE 100 //100 character limit, last one for null, use realloc to dynamically increase memory
#define PLUGBOARD_SIZE 39 //13 pairs max, 26 characters, 12 commas, 1 null char --> 39
#define SETTINGS_SIZE 8 // 8 chars - reflector, 3 rotors, 3 starting positions, and one null char

//Reflectors A - C at indexes 0 - 3
//The regular alphabet is from index 0 to 25 to reflect A to Z, the indexes will be used as input, the value at index as the output
int reflectors[3][26] = {
    {4, 9, 12, 25, 0, 11, 24, 23, 21, 1, 22, 5, 2, 17, 16, 20, 14, 13, 18, 19, 15, 8, 10, 7, 6, 3},
    {24, 17, 20, 7, 16, 18, 11, 3, 15, 23, 13, 6, 14, 10, 12, 8, 4, 1, 5, 25, 2, 22, 21, 9, 0, 19},
    {5, 21, 15, 9, 8, 0, 14, 24, 4, 3, 17, 25, 23, 22, 6, 2, 19, 10, 20, 16, 18, 1, 13, 12, 7, 11}
};

//Numerical representation of alphabet for each rotor, from 1 - 5 at indexes 0 - 4
int rotors[5][26][2] = {
    {{0, 4}, {1, 10}, {2, 12}, {3, 5}, {4, 11}, {5, 6}, {6, 3}, {7, 16}, {8, 21}, {9, 25}, {10, 13}, {11, 19}, {12, 14}, {13, 22}, {14, 24}, {15, 7}, {16, 23}, {17, 20}, {18, 18}, {19, 15}, {20, 0}, {21, 8}, {22, 1}, {23, 17}, {24, 2}, {25, 9}},
    {{0, 0}, {1, 9}, {2, 3}, {3, 10}, {4, 18}, {5, 8}, {6, 17}, {7, 20}, {8, 23}, {9, 1}, {10, 11}, {11, 7}, {12, 22}, {13, 19}, {14, 12}, {15, 2}, {16, 16}, {17, 6}, {18, 25}, {19, 13}, {20, 15}, {21, 24}, {22, 5}, {23, 21}, {24, 14}, {25, 4}},
    {{0, 1}, {1, 3}, {2, 5}, {3, 7}, {4, 9}, {5, 11}, {6, 2}, {7, 15}, {8, 17}, {9, 19}, {10, 23}, {11, 21}, {12, 25}, {13, 13}, {14, 24}, {15, 8}, {16, 4}, {17, 6}, {18, 22}, {19, 10}, {20, 0}, {21, 12}, {22, 20}, {23, 18}, {24, 16}, {25, 14}},
    {{0, 4}, {1, 18}, {2, 14}, {3, 21}, {4, 15}, {5, 25}, {6, 9}, {7, 0}, {8, 24}, {9, 16}, {10, 20}, {11, 8}, {12, 17}, {13, 7}, {14, 23}, {15, 11}, {16, 13}, {17, 5}, {18, 19}, {19, 6}, {20, 10}, {21, 3}, {22, 2}, {23, 12}, {24, 22}, {25, 1}},
    {{0, 21}, {1, 25}, {2, 1}, {3, 17}, {4, 6}, {5, 8}, {6, 19}, {7, 24}, {8, 20}, {9, 15}, {10, 3}, {11, 13}, {12, 7}, {13, 11}, {14, 23}, {15, 0}, {16, 22}, {17, 12}, {18, 9}, {19, 16}, {20, 14}, {21, 5}, {22, 4}, {23, 2}, {24, 10}, {25, 18}}
};

//Notches Q, E, V, J, Z for rotors 1 - 5 above
int notches[] = {16, 4, 21, 9, 25};


//Below are the settings to initialize, plugboard pair


//Plugboard, each index will contain index of its paired value, otherwise -1 to indicate no paired connection
int plugboard[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

//The 3 rotors, from right to left
int chosen_rotors[3][26]; //rotors right to left from indexes 0 to 2
int rotor_right[26];
int rotor_middle[26];
int rotor_left[26];

//The 3 notches, from right to left at indexes 0 to 2
int chosen_notches[3];

//The reflector
int reflector[26]; // Initialized when parsing settings, points to the one of 3 reflectors above

void readUserInput(char* ptr) {
    int numBlks = 1; 
    int currSize = BLKSIZE * numBlks;
    if(ptr != NULL) {
        int ch;
        int i = 0;
        while((ch = getchar()) != '\n' && ch != EOF) {
            ptr[i++] = (char) ch;

            if(i == currSize) {
                currSize = BLKSIZE * (++numBlks);
                ptr = realloc(ptr, currSize + 1);
            }
        }

        ptr[i] = '\0';
    }
}

void getUserInput(char* settings, char* connections, char* input) {
    printf("Input (ENTER '0' to terminate program):\n");
    scanf("%s", settings);
    // fgets(settings, SETTINGS_SIZE, stdin);
    if(strlen(settings) == 1) {
        return;
    }


    scanf("%s\n", connections);
    // fgets(plugBoardInput, PLUGBOARD_SIZE, stdin);
    readUserInput(input);
}

//rotates a rotor and checks if a notch has done a full circle
int rotor_movement(int* rotor, int notch) {
    int temp[] = rotor[0];
    for(int i=1; i<26; i++) {
        rotor[i - 1] = rotor[i];
    }

    memcpy(rotor, rotor[1], sizeof(*(rotor + 1)));
    memcpy(*(rotor + 25), temp, sizeof(temp));
    rotor[25] = temp;

    if(rotor[25] == notch) {
        return 1; //Notch moved back, move next rotor
    }

    return 0; //Don't move next rotor
}

//The 3 rotors and their notches, right to left
void rotate_rotors() {
    //The leftmost rotor always rotates before the input is going through the rotor
    int move = rotor_movement(chosen_rotors[2], chosen_notches[2]); //returns 1 where if notch went from 0th index to last index indicates move the next rotor, else 0 to not move the next rotor
    if(move == 1) {
        move = rotor_movement(chosen_rotors[1], chosen_notches[1]); //likewise, return 1 if middle notch goes from front to Back, else 0

        if(move == 1) {
            rotor_movement(chosen_rotors[0], chosen_notches[0]);
        }
    }
}

//rotates each rotor until it matches starting position
void set_starting_pos(int* rotor, int notch, char target) {
    int num = target - 'A';
    printf("%d\n", num);
    while(rotor[0] != num) {
        rotor_movement(rotor, notch);
    }
}

void parse_settings(char* settings) {
    //initialize the reflector to use
    switch(settings[0]) {
        case 'A': 
            memcpy(reflector, reflectors[0], sizeof(reflector));
            break;
        case 'B':
            memcpy(reflector, reflectors[1], sizeof(reflector));
            break;
        case 'C':
            memcpy(reflector, reflectors[2], sizeof(reflector));
            break;
        default:
            break;
    }

    //initialize the 3 rotors to use & track their notches
    for(int i=1; i<4; i++) {
        switch(settings[i]) {
            case '1': 
                memcpy(chosen_rotors[i - 1], rotors[0], sizeof(rotors[0]));
                chosen_notches[i - 1] = notches[0];
                break;
            case '2':
                memcpy(chosen_rotors[i - 1], rotors[1], sizeof(rotors[1]));
                chosen_notches[i - 1] = notches[1];
                break;
            case '3':
                memcpy(chosen_rotors[i - 1], rotors[2], sizeof(rotors[2]));
                chosen_notches[i - 1] = notches[2];
                break;
            case '4':
                memcpy(chosen_rotors[i - 1], rotors[3], sizeof(rotors[3]));
                chosen_notches[i - 1] = notches[3];
                break;
            case '5':
                memcpy(chosen_rotors[i - 1], rotors[4], sizeof(rotors[4]));
                chosen_notches[i - 1] = notches[4];
                break;
            default:
                break; 
        }
    }

    // initialize the 3 rotors starting position
    for(int i=4; i<7; i++) {
        set_starting_pos(chosen_rotors[i - 4], chosen_notches[i - 4], settings[i]);
    }

    // for(int i=0; i<26; i++) {
    //     printf("%d ", reflector[i]);
    // }

    // printf("\n");

    // for(int i=0; i<26; i++) {
    //     printf("%d ", chosen_rotors[0][i]);
    // }

    // printf("\n");

    // for(int i=0; i<26; i++) {
    //     printf("%d ", chosen_rotors[1][i]);
    // }

    // printf("\n");

    // for(int i=0; i<26; i++) {
    //     printf("%d ", chosen_rotors[2][i]);
    // }

    // printf("\n");
}

void set_plugboard(char* conn) {
    int pair_1, pair_2;
    char delim = ',';
    for(int i=0; i<strlen(conn); i++) {
        pair_1 = conn[i++] - 'A';
        pair_2 = conn[i++] - 'A';
        plugboard[pair_1] = pair_2;
        plugboard[pair_2] = pair_1;
    }

    // for(int i=0; i<26; i++) {
    //     printf("%d ", plugboard[i]);
    // }

    // printf("\n");
}

int through_plugboard(int num) {
    if(plugboard[num] != -1) {
        return plugboard[num];
    }

    return num;
}

// direction - 0 means normal direction, 1 is reverse direction after the reflector
int through_rotor(int* rotor, int num, int direction) {
    if(direction == 0) { //only while going through rotors normal direction
        rotate_rotors();
    }

    return rotor[num];
}

void start_process(char* in, char* out) {
    int num;
    char ch;
    for(int i=0; i<strlen(in); i++) {
        num = in[i] - 'A'; //character
        
        num = through_plugboard(num); //go through plugboard
        num = through_rotor(chosen_rotors[2], num, 0); //through left rotor
        num = through_rotor(chosen_rotors[1], num, 0); //through middle rotor
        num = through_rotor(chosen_rotors[0], num, 0); //through right rotor
        num = reflector[num]; //through reflector
        num = through_rotor(chosen_rotors[0], num, 1); //through right rotor        
        num = through_rotor(chosen_rotors[1], num, 1); //through middle rotor
        num = through_rotor(chosen_rotors[2], num, 1); //through left rotor
        num = through_plugboard(num); //go through plugboard
        
        ch = num + 'A';
        out[i] = ch;
    }
}

int main() {
    int terminate = 0; // terminate: 0 - continue running program, 1 - end program
    char* settings = malloc(SETTINGS_SIZE); //7 characters + 1 null char
    char* plugBoardInput = malloc(PLUGBOARD_SIZE); //There can only be maximum 13 pairs, 12 commas

    do {
        char* userInput = malloc(BLKSIZE + 1);

        getUserInput(settings, plugBoardInput, userInput);
        if(strlen(settings) == 1) { //for termination
            free(settings);
            free(plugBoardInput);
            free(userInput);
            return 0;
        }

        parse_settings(settings);
        set_plugboard(plugBoardInput);

        char* output = malloc(strlen(userInput) + 1);
        output[strlen(userInput)] = '\0';

        //Does the encryption / decryption
        start_process(userInput, output);

        //Now, we have the expected value, print it out
        printf("%s\n", output);

        free(output);
        free(userInput);
    } while(!terminate);

    free(settings);
    free(plugBoardInput);
    return 0;
}