#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;
int n = 0;


struct FileNode {
    std::string fileName;
    std::string filePath;
    FileNode* left;
    FileNode* right;

    FileNode(const std::string& fileName, const std::string& filePath)
        : fileName(fileName), filePath(filePath), left(nullptr), right(nullptr) {}
};

class FileIndex {
private:
    FileNode* root;
    std::unordered_map<std::string, std::string> fileMap; // Map to track files for duplication

    // Utility to insert nodes into the BST
    FileNode* insert(FileNode* node, const std::string& fileName, const std::string& filePath) {
        if (node == nullptr) {
            return new FileNode(fileName, filePath);
        }

        if (fileName < node->fileName) {
            node->left = insert(node->left, fileName, filePath);
        } else if (fileName > node->fileName) {
            node->right = insert(node->right, fileName, filePath);
        } else if (fileName == node->fileName) {
	    std::cout << "File " << node->fileName << "already exists" << std::endl;
	    n = 1;
	}
        return node;
    }

    // Utility to search for a file in BST
    FileNode* search(FileNode* node, const std::string& fileName) {
        if (node == nullptr || node->fileName == fileName) {
            return node;
        }
        if (fileName < node->fileName) {
            return search(node->left, fileName);
        }
        return search(node->right, fileName);
    }

    // Utility to find duplicates based on file path
    void findDuplicates(FileNode* node) {
        if (node == nullptr) return;

        if (fileMap.find(node->filePath) != fileMap.end()) {
            std::cout << "Duplicate found: " << node->fileName << " at " << node->filePath << std::endl;
        } else {
            fileMap[node->filePath] = node->fileName;
        }

        findDuplicates(node->left);
        findDuplicates(node->right);
    }

    // Utility to delete a file node
    FileNode* deleteNode(FileNode* root, const std::string& fileName) {
        if (root == nullptr) return root;

        if (fileName < root->fileName) {
            root->left = deleteNode(root->left, fileName);
        } else if (fileName > root->fileName) {
            root->right = deleteNode(root->right, fileName);
        } else {
            if (root->left == nullptr) {
                FileNode* temp = root->right;
                delete root;
                return temp;
            } else if (root->right == nullptr) {
                FileNode* temp = root->left;
                delete root;
                return temp;
            }

            FileNode* temp = minValueNode(root->right);
            root->fileName = temp->fileName;
            root->filePath = temp->filePath;
            root->right = deleteNode(root->right, temp->fileName);
        }
        return root;
    }

    FileNode* minValueNode(FileNode* node) {
        FileNode* current = node;
        while (current && current->left != nullptr) {
            current = current->left;
        }
        return current;
    }

    // Utility to save the index data to a file
    void saveIndexToFile(FileNode* node, std::ofstream& outFile) {
        if (node == nullptr) return;
        outFile << node->fileName << " " << node->filePath << std::endl;
        saveIndexToFile(node->left, outFile);
        saveIndexToFile(node->right, outFile);
    }

    // Utility to load index data from a file
    void loadIndexFromFile(std::ifstream& inFile) {
        std::string fileName, filePath;
        while (inFile >> fileName >> filePath) {
            root = insert(root, fileName, filePath);
        }
    }

public:
    FileIndex() : root(nullptr) {}

    // Function to add a file to the index
    void addFile(const std::string& fileName, const std::string& filePath) {
        root = insert(root, fileName, filePath);
    }

    // Function to remove a file from the index
    void removeFile(const std::string& fileName) {
        root = deleteNode(root, fileName);
    }

    // Function to find duplicates
    void findDuplicates() {
        findDuplicates(root);
    }

    // Function to save the index to a file
    void saveIndex(const std::string& fileName) {
        std::ofstream outFile(fileName);
        saveIndexToFile(root, outFile);
    }

    // Function to load the index from a file
    void loadIndex(const std::string& fileName) {
        std::ifstream inFile(fileName);
        loadIndexFromFile(inFile);
    }

    // Function to display the index
    void displayIndex(FileNode* node) {
        if (node == nullptr) return;
        displayIndex(node->left);
        std::cout << node->fileName << " " << node->filePath << std::endl;
        displayIndex(node->right);
    }

    FileNode* getRoot() { return root; }
};

int main(int argc, char* argv[]) {
    FileIndex fileIndex;
    std::string indexFileName = "fileIndex.txt";

    // Load existing index from file
    fileIndex.loadIndex(indexFileName);

    if (argc < 3) {
        std::cerr << "Usage: <command> <file_name> [<file_path>]\n";
        return 1;
    }

    std::string command = argv[1];
    std::string fileName = argv[2];
    std::string filePath = (argc > 3) ? argv[3] : "";

    if (command == "add" && !filePath.empty()) {
        fileIndex.addFile(fileName, filePath);
	if (n == 0) {
        	std::cout << "Added file: " << fileName << " at " << filePath << std::endl;
	}
	n = 0;
    } else if (command == "remove") {
        fileIndex.removeFile(fileName);
        std::cout << "Removed file: " << fileName << std::endl;
    } else {
        std::cerr << "Invalid command or missing arguments.\n";
    }
	
    fileIndex.findDuplicates();
    // Save updated index to file
    fileIndex.saveIndex(indexFileName);

    return 0;
}