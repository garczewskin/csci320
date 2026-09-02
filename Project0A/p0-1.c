#include <stdio.h>
#include <ctype.h>

// Struct to store letters and their counts
struct Letter {
    char letter;
    int count;
};

// Function to count each letter passed to it
void countLetters(struct Letter letters[], char ch) {
    
    ch = tolower(ch); // Convert any uppercase letter to lowercase

    // Strip out any non-letter characters
    if(ch >= 'a' && ch <= 'z') {
        
        // Increment that letter's count. 
        // Since 'a' is at position 0, the offset from 'a' is used to find the letter's position in the array
        letters[ch - 'a'].count++;
    
    }
}

int main(void) {

    // Create an array to hold each letter of the alphabet and its count
    struct Letter letters[26];

    // Initialize the array
    for(int i = 0; i < 26; i++) {
        letters[i].letter = 'a' + i; // Initialize first item to 'a' and increment from there to fill out the rest of the alphabet
        letters[i].count = 0; // Initialize every letter's count to zero
    }

    // Read input character-by-character
    while (!feof(stdin)) {
        char ch;
        scanf("%c",&ch);
        countLetters(letters, ch);
    }

    // Print the contents of the array
    for (int j = 0; j < 26; j++)
    {
        printf("%c: %d\n", letters[j].letter, letters[j].count);
    }

    return 0;

}