using namespace std;

int main () {
    int len = 27;
    int y;

    for (int i = 0; i < len; i++) {
        int x = i % 2;
        if (x == 0) y = 1;
        else if (x == 1) y = 2;
    }

    return 0;
}
