using namespace std;

int main () {
    int len = 27;
    int y;

    for (int j = 0; j < len; j++) {
        for (int i = 0; i < len; i++) {
            int x = i % 2 + j;
            if (x == 0 && j > 0) y = 1 + j;
            else if (x == 1) y = 2;
        }
    }

    return 0;
}
