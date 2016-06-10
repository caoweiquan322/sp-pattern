/* Copyright © 2015 DynamicFatty. All Rights Reserved. */

#ifndef TRIENODE
#define TRIENODE

template<class T>
class TrieNode {
public:
    // The constructor.
    TrieNode() {
        element = 0;
    }
    TrieNode(T e) {
        element = e;
    }

    TrieNode(const TrieNode &other) {
        this->element = other.element;
        this->children = other.children;
    }

    // The de-constructor.
    ~TrieNode() {
        foreach (TrieNode *child, children) {
            delete child;
        }
        children.clear();
    }

    // Insert a string into the trie.
    void insert(const QVector<T> &string, int from) {
        if (from<0 || from>=string.count()) {
            return;
        }

        // Checking if there's a child matching string[from].
        T e = string.at(from);
        foreach (TrieNode *child, children) {
            if (child->element == e) {
                return child->insert(string, from+1);
            }
        }
        // Matched child not found.
        TrieNode *parent = this, *node;
        for (int i=from; i<string.count(); ++i) {
            e = string.at(i);
            node = new TrieNode(e);
            parent->children.insert(node);
            parent = node;
        }
    }

    // Collect all the leaves.
    void collectLeaves(QVector<QVector<T> > &allLeaves) {
        QVector<T> prefix;
        _collectLeaves(prefix, allLeaves);
    }

protected:
    // This method does all the work.
    void _collectLeaves(QVector<T> &prefix,
                       QVector<QVector<T> > &allLeaves) {
        // Checking if this is a leaf.
        if (children.isEmpty()) {
            if (!prefix.isEmpty()) {
                allLeaves.append(prefix);
                prefix.removeLast();
            }
            return;
        }

        // Run with its children recursively.
        foreach (TrieNode *child, children) {
            prefix.append(child->element);
            child->_collectLeaves(prefix, allLeaves);
        }
        if (!prefix.isEmpty())
            prefix.removeLast();
    }

protected:
    T element;
    QSet<TrieNode *> children;
};

#endif // TRIENODE

