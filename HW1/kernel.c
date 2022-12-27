void k_print(char *string, int string_length, int row, int column);
void k_clearscr();
void print_border(int start_row, int start_column, int end_row, int end_column);
char *message = "OS 4100";
char *topbotom = "+------------------------------------------------------------------------------+";
char *middle = "|                                                                              |";
char *blank = "                                                                                ";

void main() {
    k_clearscr();
    print_border(0,0,24,79);
    k_print(message, 7, 12, 39);
    while(1);
}

void k_clearscr() {
    for(int i = 0; i < 25;i++) {
            k_print(blank, 80,i,0);
    }
}

void print_border(int start_row, int start_column, int end_row, int end_column) {
    int startR = start_row + 1;
    k_print(topbotom, 80, start_row, start_column);
    for(startR; startR < 24; startR++) {
        k_print(middle, 80, startR, 0);
    }
    k_print(topbotom, 80, end_row, start_column);
}