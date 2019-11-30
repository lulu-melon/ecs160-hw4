#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_LINE_LEN 1024
#define MAX_FILE_LEN 20000

/*
 * calculates the top 10 tweeters (by # of tweets) in a given CSV file of tweets
 * takes one command line argument, the path of the CSV file
 * output (for valid input) is in the form of <tweeter>: <count of tweets> listed in decreasing order
 * for invalid input, output is: Invalid Input Format
 */

struct MapEntry {
    char *tweeter;
    int num_tweets;
};

void merge(struct MapEntry *map[], int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;
    struct MapEntry *arr1[n1];
    struct MapEntry *arr2[n2];
    for (int i = 0; i < n1; i++) {
        arr1[i] = map[l + i];
    }
    for (int j = 0; j < n2; j++) {
        arr2[j] = map[m + 1 + j];
    }
    int i = 0;
    int j = 0;
    int k = l;
    while (i < n1 && j < n2) {
        if (arr1[i]->num_tweets < arr2[j]->num_tweets) {
            map[k] = arr2[j];
            j++;
        }
        else {
            map[k] = arr1[i];
            i++;
        }
        k++;
    }
    while (i < n1) {
        map[k] = arr1[i];
        k++;
        i++;
    }
    while (j < n2) {
        map[k] = arr2[j];
        k++;
        j++;
    }
}

// sorts the MapEntry instances by number of tweets in decreasing order
void mergeSort(struct MapEntry *map[], int l, int r) {
    if (l < r) {
        int m = (l + r) / 2;
        mergeSort(map, l, m);
        mergeSort(map, m + 1, r);
        merge(map, l, m, r);
    }
}

/*
 * returns the field in line beginning at index *i
 * the new value of *i will be the starting index of the next field, or -1 if none exists
 * *len will store the length of the current field
 * *lineExceedsMaxLen will be set to true iff the length of the line (including the '\n' character) exceeds 1024 characters
 */
char * linetok(char * const line, int *i, int *len, bool *lineExceedsMaxLen) {
    int j = *i;
    while (j < MAX_LINE_LEN && line[j] != ',' && line[j] != '\n' && line[j] != '\0') {
        j++;
    }
    if (j == MAX_LINE_LEN) {
        *lineExceedsMaxLen = true;
    }
    char *token = (char*) malloc(sizeof(char) * (j - *i + 1));
    for (int k = *i; k < j; k++) {
        *token = line[k];
        token++;
    }
    *token = '\0';
    token = token - (j - *i);
    *len = j - *i;
    if (line[j] == '\n' || line[j] == '\0') {
        *i = -1;
    }
    else {
        *i = j + 1;
    }
    return token;
}

/*
 * returns:
 *  1, if the field is surrounded by quotes
 *  0, if not
 *  -1, for invalid quote formatting
 */
int isQuoted(char * const token, int len) {
    if (len == 0) return false;
    else if (len == 1) {
        if (token[0] == '"') return -1;
        else return false;
    }
    else {
        if (token[0] == '"' && token[len - 1] == '"') return true;
        else if (token[0] == '"' || token[len - 1] == '"') return -1;
        else return false;
    }
}

/*
 * returns:
 *  the index of the last character in the header, if the header is valid
 *  -1, if the header is invalid (see possible cases below)
 *      --> the length of the line exceeds 1024 characters
 *      --> line contains invalid quote formatting
 *      --> line contains more than one "name" field
 * at the end of the function call, the value of *field_index will be "the number of fields in line" - 1
 * for each field i, quoted[i] will be set to true iff i is surrounded by quotes, and false otherwise
 * if a "name" field is found, then *name_index will store its position relative to the other fields (position numbering starts from 0)
 */
int readHeader(char *line, int *field_index, bool *quoted, int *name_index) {
    int index = 0;
    int len = 0;
    bool lineExceedsMaxLen = false;
    int last_index = 0;
    while (index != -1) {
        last_index = index;
        char *token = linetok(line, &index, &len, &lineExceedsMaxLen);
        last_index += len;
        if (lineExceedsMaxLen) return -1;
        int fieldIsQuoted = isQuoted(token, len);
        if (fieldIsQuoted == -1) return -1;
        else quoted[*field_index] = fieldIsQuoted == 1 ? true : false;
        if (strcmp(token, "name") == 0 || strcmp(token, "\"name\"") == 0) {
            if (*name_index == -1) {
                // found first occurrence of "name" in the header
                *name_index = *field_index;
            }
            else {
                // there are multiple occurrences of "name" in the header
                return -1;
            }
        }
        (*field_index)++;
        free(token);
    }
    return last_index;
}

/*
 * returns:
 *  true, if quote formatting is valid AND (the field should be and is surrounded by quotes OR the field should not be and is not surrounded by quotes)
 *  false, otherwise
 */
bool quotesMatch(char * const token, int len, bool * const quoted, int i) {
    if (len == 0 && quoted[i]) return false;
    else if (len == 1 && (quoted[i] || token[0] == '"')) return false;
    else if (len > 1 && quoted[i] && (token[0] != '"' || token[len - 1] != '"')) return false;
    else if (len > 1 && !quoted[i] && (token[0] == '"' || token[len - 1] == '"')) return false;
    else return true;
}

/*
 * increments the number of tweets in the tweeter's MapEntry
 * *map_index will store the index of the most recently added MapEntry in map
 */
void addToMap(char *token, struct MapEntry *map[], int *map_index) {
    bool found = false;
    for (int j = 0; j <= *map_index; j++) {
        if (strcmp(map[j]->tweeter, token) == 0) {
            // entry for this tweeter already exists
            found = true;
            map[j]->num_tweets++;
            break;
        }
    }
    if (!found) {
        // create new entry in map
        (*map_index)++;
        struct MapEntry *entry = malloc(sizeof(struct MapEntry));
        entry->tweeter = malloc(strlen(token) + 1);
        strcpy(entry->tweeter, token);
        entry->num_tweets = 1;
        map[*map_index] = entry;
    }
}

/*
 * returns:
 *  the index of the last character in the line, if the line is valid
 *  -1, if invalid input is encountered
 */
int readLine(char *line, int *line_num, bool *quoted, int name_index, struct MapEntry *map[], int *map_index, int field_index) {
    if (++(*line_num) == MAX_FILE_LEN) return -1; // the length of the file exceeds 20,000 lines
    int index = 0;
    int len = 0;
    bool lineExceedsMaxLen = false;
    int last_index = 0;
    int i = 0;
    while (index != -1) {
        if (i > field_index) return -1; // the line contains more fields than the header does
        last_index = index;
        char *token = linetok(line, &index, &len, &lineExceedsMaxLen);
        last_index += len;
        if (lineExceedsMaxLen || !quotesMatch(token, len, quoted, i)) return -1;
        if (i == name_index) {
            // this token contains the name of the tweeter
            // remove quotes if applicable
            if (quoted[name_index]) {
                token[len - 1] = '\0';
                addToMap(token + 1, map, map_index);
            }
            else addToMap(token, map, map_index);
        }
        i++;
        free(token);
    }
    // the condition below checks whether this line has the same number of fields as the header does
    if (i != field_index) return -1;
    else return last_index;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Invalid Input Format\n");
        return -1;
    }
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        // encountered error while opening the file
        printf("Invalid Input Format\n");
        return -1;
    }

    // read file header
    char line[MAX_LINE_LEN + 1];
    if (fgets(line, MAX_LINE_LEN + 1, file) == NULL) {
        // missing header
        printf("Invalid Input Format\n");
        return -1;
    }
    int field_index = 0;
    bool quoted[MAX_LINE_LEN + 1];
    int name_index = -1;
    int last_index = readHeader(line, &field_index, quoted, &name_index);
    char last_read_char;
    if (name_index == -1 || last_index == -1) {
        // did not find an occurrence of "name" in the header OR encountered some other form of invalid input
        printf("Invalid Input Format\n");
        return -1;
    }
    else last_read_char = line[last_index];

    struct MapEntry *map[MAX_FILE_LEN];
    int map_index = -1;
    int line_num = 0;

    // read the remaining lines in the file
    while (fgets(line, MAX_LINE_LEN + 1, file) != NULL) {
        last_index = readLine(line, &line_num, quoted, name_index, map, &map_index, field_index);
        if (last_index == -1) {
            printf("Invalid Input Format\n");
            return -1;
        }
        else last_read_char = line[last_index];
    }
    if (last_read_char == '\n') {
        // read the last line of the file
        last_index = readLine(&last_read_char, &line_num, quoted, name_index, map, &map_index, field_index);
        if (last_index == -1) {
            printf("Invalid Input Format\n");
            return -1;
        }
    }

    // sort the map in decreasing order, then print the first 10 entries
    mergeSort(map, 0, map_index);
    for (int i = 0; i < 10; i++) {
        if (i <= map_index) {
            printf("%s: %d\n", map[i]->tweeter, map[i]->num_tweets);
        }
    }

    for (int i = 0; i < MAX_FILE_LEN; i++) {
        if (map[i] != NULL) {
            free(map[i]->tweeter);
            free(map[i]);
        }
        else break;
    }

    fclose(file);
    return 0;
}