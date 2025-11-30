#include <bits/stdc++.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
using namespace std;

#define MAX 256

// ---------------- STRUCTURES ----------------
struct Node {
    char character;
    int freq;
    Node *l, *r;
    Node(char c, int f) : character(c), freq(f), l(nullptr), r(nullptr) {}
};

struct Compare {
    bool operator()(Node* l, Node* r) {
        return l->freq > r->freq;
    }
};

struct Code {
    char k;
    int l;
    vector<int> code_arr;
};

// For decoding
struct Tree {
    char g;
    int len;
    int dec;
    Tree *f, *r;
};

// ---------------- GLOBALS ----------------
vector<Code> codes;
Tree* rootTree = nullptr;

// ---------------- UTILITIES ----------------
bool isLeaf(Node* root) {
    return !(root->l) && !(root->r);
}

void convertDecimalToBinary(int arr[], int num, int len) {
    for (int i = len - 1; i >= 0; i--) {
        arr[i] = num % 2;
        num /= 2;
    }
}

int convertBinaryToDecimal(vector<int>& code) {
    int val = 0;
    for (int bit : code)
        val = (val << 1) | bit;
    return val;
}

// ---------------- BUILD HUFFMAN TREE ----------------
Node* buildHuffmanTree(char arr[], int freq[], int n) {
    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (int i = 0; i < n; ++i)
        pq.push(new Node(arr[i], freq[i]));

    while (pq.size() > 1) {
        Node *l = pq.top(); pq.pop();
        Node *r = pq.top(); pq.pop();
        Node* top = new Node('$', l->freq + r->freq);
        top->l = l;
        top->r = r;
        pq.push(top);
    }
    return pq.top();
}

// ---------------- GENERATE HUFFMAN CODES ----------------
void storeCodes(Node* root, vector<int>& arr, int fd2) {
    if (!root) return;

    if (root->l) {
        arr.push_back(0);
        storeCodes(root->l, arr, fd2);
        arr.pop_back();
    }
    if (root->r) {
        arr.push_back(1);
        storeCodes(root->r, arr, fd2);
        arr.pop_back();
    }

    if (isLeaf(root)) {
        Code data;
        data.k = root->character;
        data.l = arr.size();
        data.code_arr = arr;
        codes.push_back(data);

        int dec = convertBinaryToDecimal(arr);
        write(fd2, &data.k, sizeof(char));
        write(fd2, &data.l, sizeof(int));
        write(fd2, &dec, sizeof(int));

        cout << root->character << ": ";
        for (int b : arr) cout << b;
        cout << "\n";
    }
}

// ---------------- FILE COMPRESSION ----------------
void compressFile(int fd1, int fd2) {
    unsigned char a = 0;
    int h = 0;
    char n;

    while (read(fd1, &n, sizeof(char)) != 0) {
        for (auto &c : codes) {
            if (c.k == n) {
                for (int bit : c.code_arr) {
                    a = (a << 1) | bit;
                    h++;
                    if (h == 8) {
                        write(fd2, &a, sizeof(char));
                        a = 0;
                        h = 0;
                    }
                }
            }
        }
    }
    if (h > 0) {
        a <<= (8 - h);
        write(fd2, &a, sizeof(char));
    }
}

// ---------------- FILE DECOMPRESSION ----------------
void rebuildTreeFromCodes(vector<Code>& codes) {
    rootTree = new Tree();
    for (auto &c : codes) {
        Tree* temp = rootTree;
        for (int bit : c.code_arr) {
            if (bit == 0) {
                if (!temp->f) temp->f = new Tree();
                temp = temp->f;
            } else {
                if (!temp->r) temp->r = new Tree();
                temp = temp->r;
            }
        }
        temp->g = c.k;
    }
}

void decompressFile(int fd1, int fd2, Node* root, int totalChars) {
    Tree* temp = rootTree;
    unsigned char byte;
    int bits[8];
    int count = 0;

    while (read(fd1, &byte, sizeof(char)) != 0) {
        for (int i = 7; i >= 0; --i)
            bits[7 - i] = (byte >> i) & 1;

        for (int i = 0; i < 8 && count < totalChars; i++) {
            temp = (bits[i] == 0) ? temp->f : temp->r;
            if (temp && !temp->f && !temp->r) {
                write(fd2, &temp->g, sizeof(char));
                temp = rootTree;
                count++;
            }
        }
    }
}

// ---------------- MAIN FUNCTION ----------------
int main() {
    int fd1 = open("sample.txt", O_RDONLY);
    if (fd1 == -1) {
        perror("❌ Error: 'sample.txt' not found.");
        return 1;
    }

    int fd2 = open("sample-compressed.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd2 == -1) {
        perror("❌ Error creating compressed file");
        return 1;
    }

    unordered_map<char, int> freq;
    char ch;
    while (read(fd1, &ch, sizeof(char)) > 0) freq[ch]++;
    close(fd1);

    vector<char> chars;
    vector<int> freqs;
    for (auto &p : freq) {
        chars.push_back(p.first);
        freqs.push_back(p.second);
    }

    Node* root = buildHuffmanTree(chars.data(), freqs.data(), chars.size());
    cout << "📘 Huffman Codes:\n";

    vector<int> arr;
    storeCodes(root, arr, fd2);
    close(fd2);

    fd1 = open("sample.txt", O_RDONLY);
    fd2 = open("compressed.bin", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    compressFile(fd1, fd2);
    close(fd1);
    close(fd2);

    cout << "\n✅ Compression done.\n📦 Compressed file: compressed.bin\n";

    // Rebuild Huffman tree from stored codes
    rebuildTreeFromCodes(codes);

    fd1 = open("compressed.bin", O_RDONLY);
    fd2 = open("sample-decompressed.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    decompressFile(fd1, fd2, root, 0);
    close(fd1);
    close(fd2);

    cout << "📄 Decompressed file: sample-decompressed.txt\n";
    return 0;
}
