#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <openssl/sha.h>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

struct BSTNode {
    std::string fileName;
    std::string fileHash;
    std::string filePath;
    BSTNode* left;
    BSTNode* right;

    BSTNode(std::string name, std::string hash, std::string path)
        : fileName(name), fileHash(hash), filePath(path), left(nullptr), right(nullptr) {}
};

// Function to calculate file hash (SHA256)
std::string calculateFileHash(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) return "";

    std::ostringstream oss;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    char buffer[8192];

    while (file.read(buffer, sizeof(buffer))) {
        SHA256_Update(&sha256, buffer, file.gcount());
    }
    SHA256_Update(&sha256, buffer, file.gcount());
    SHA256_Final(hash, &sha256);

    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        oss << std::hex << static_cast<int>(hash[i]);
    }
    return oss.str();
}

// Insert a node into the BST
BSTNode* insertNode(BSTNode* root, BSTNode* node) {
    if (root == nullptr) {
        return node;
    }
    if (node->fileHash < root->fileHash) {
        root->left = insertNode(root->left, node);
    } else {
        root->right = insertNode(root->right, node);
    }
    return root;
}

// Find and delete a node in the BST
BSTNode* deleteNode(BSTNode* root, const std::string& fileHash) {
    if (root == nullptr) return root;

    if (fileHash < root->fileHash) {
        root->left = deleteNode(root->left, fileHash);
    } else if (fileHash > root->fileHash) {
        root->right = deleteNode(root->right, fileHash);
    } else {
        if (root->left == nullptr) {
            BSTNode* temp = root->right;
            delete root;
            return temp;
        } else if (root->right == nullptr) {
            BSTNode* temp = root->left;
            delete root;
            return temp;
        }

        BSTNode* temp = root->right;
        while (temp && temp->left != nullptr) {
            temp = temp->left;
        }
        root->fileName = temp->fileName;
        root->fileHash = temp->fileHash;
        root->filePath = temp->filePath;
        root->right = deleteNode(root->right, temp->fileHash);
    }
    return root;
}

// Save BST to a file
void saveBST(std::ofstream& outFile, BSTNode* root) {
    if (root == nullptr) return;

    outFile << root->fileName << "|" << root->fileHash << "|" << root->filePath << "\n";
    saveBST(outFile, root->left);
    saveBST(outFile, root->right);
}

// Load BST from a file
BSTNode* loadBST(std::ifstream& inFile) {
    BSTNode* root = nullptr;
    std::string line;
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        std::string fileName, fileHash, filePath;
        std::getline(iss, fileName, '|');
        std::getline(iss, fileHash, '|');
        std::getline(iss, filePath, '|');
        root = insertNode(root, new BSTNode(fileName, fileHash, filePath));
    }
    return root;
}

// Find duplicate files
void findDuplicates(BSTNode* root, std::unordered_map<std::string, std::vector<std::string>>& hashMap) {
    if (root == nullptr) return;

    hashMap[root->fileHash].push_back(root->filePath);
    findDuplicates(root->left, hashMap);
    findDuplicates(root->right, hashMap);
}

// Print duplicate files
void printDuplicates(const std::unordered_map<std::string, std::vector<std::string>>& hashMap) {
    for (const auto& [hash, paths] : hashMap) {
        if (paths.size() > 1) {
            std::cout << "Duplicate files (Hash: " << hash << "):\n";
            for (const auto& path : paths) {
                std::cout << "  " << path << "\n";
            }
        }
    }
}

// User Menu
void userMenu(BSTNode*& root) {
    int choice;
    std::string filePath;
    do {
        std::cout << "\nFile Indexing Application\n";
        std::cout << "1. Add File\n";
        std::cout << "2. Remove File\n";
        std::cout << "3. Find Duplicates\n";
        std::cout << "4. Save Index\n";
        std::cout << "5. Load Index\n";
        std::cout << "6. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1:
                std::cout << "Enter file path: ";
                std::cin >> filePath;
                if (fs::exists(filePath)) {
                    std::string hash = calculateFileHash(filePath);
                    root = insertNode(root, new BSTNode(fs::path(filePath).filename().string(), hash, filePath));
                    std::cout << "File added.\n";
                } else {
                    std::cout << "File does not exist.\n";
                }
                break;
            case 2:
                std::cout << "Enter file path to remove: ";
                std::cin >> filePath;
                if (fs::exists(filePath)) {
                    std::string hash = calculateFileHash(filePath);
                    root = deleteNode(root, hash);
                    std::cout << "File removed.\n";
                } else {
                    std::cout << "File does not exist.\n";
                }
                break;
            case 3: {
                std::unordered_map<std::string, std::vector<std::string>> hashMap;
                findDuplicates(root, hashMap);
                printDuplicates(hashMap);
                break;
            }
            case 4: {
                std::ofstream outFile("index.dat");
                saveBST(outFile, root);
                outFile.close();
                std::cout << "Index saved.\n";
                break;
            }
case 5: {
                std::ifstream inFile("index.dat");
                root = loadBST(inFile);
                inFile.close();
                std::cout << "Index loaded.\n";
                break;
            }
            case 6:
                std::cout << "Exiting...\n";
                break;
            default:
                std::cout << "Invalid choice.\n";
        }
    } while (choice != 6);
}

// Main Function
int main() {
    BSTNode* root = nullptr;
    userMenu(root);
    return 0;
}
