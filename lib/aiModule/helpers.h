#ifndef HELPERS_H
#define HELPERS_H


String joinStringArray(String arr[], size_t len, const char delimiter);
int splitString(const String& input, char delimiter, String output[], int maxParts);


// Helper function to join array of strings into one string with delimiter
String joinStringArray(String arr[], size_t len, const char delimiter) {
    String result = "";
    for (size_t i = 0; i < len; i++) {
        result += arr[i];
        if (i < len - 1) {
        result += delimiter;
        }
    }
    return result;
}

int splitString(const String& input, char delimiter, String output[], int maxParts) {
    int partIndex = 0;
    int startIndex = 0;
    int delimIndex;

    while (partIndex < maxParts - 1) {
        delimIndex = input.indexOf(delimiter, startIndex);
        if (delimIndex == -1) break;

        output[partIndex] = input.substring(startIndex, delimIndex);
        startIndex = delimIndex + 1;
        partIndex++;
    }
    output[partIndex] = input.substring(startIndex); // last part
    return partIndex + 1; // total parts found
}

#endif //HELPERS_H