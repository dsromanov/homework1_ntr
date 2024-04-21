#include <iostream>
#include <memory>
#include <stdexcept>

template<typename T>
class SingleLinkedList {
private:
    struct Node {
        T data;
        std::shared_ptr<Node> next;
        explicit Node(const T& value) : data(value), next(nullptr) {}
    };

    std::shared_ptr<Node> head;
    std::shared_ptr<Node> tail;

public:
    SingleLinkedList() = default;

    ~SingleLinkedList() {
        clear();
    }


    void push_back(const T& value) {
        std::shared_ptr<Node> new_node = std::make_shared<Node>(value);

        if (!head) {
            head = new_node;
            tail = new_node;
        } else {
            tail->next = new_node;
            tail = new_node;
        }
    }

    void push_front(const T& value) {
        std::shared_ptr<Node> new_node = std::make_shared<Node>(value);

        if (!head) {
            head = new_node;
            tail = new_node;
        } else {
            new_node->next = head;
            head = new_node;
        }
    }

    std::shared_ptr<Node> search(const T& value) const{
        std::shared_ptr<Node> curr = head;
        while (curr) {
            if (curr->data == value) {
                return curr;
            }
            curr = curr->next;
        }
        return nullptr;
    }

    bool contains(const T& value) const{
        return search(value) != nullptr;
    }

    void remove(const T& value) {
        if (!head) {
            return;
        }

        if (head->data == value) {
            head = head->next;
            return;
        }

        std::shared_ptr<Node> prev = head;
        std::shared_ptr<Node> curr = head->next;

        while (curr) {
            if (curr->data == value) {
                prev->next = curr->next;
                return;
            }
            prev = curr;
            curr = curr->next;
        }
    }

    T back() {
        if (!tail) {
            throw std::out_of_range("List is empty");
        }
        return tail->data;
    }

    T front() const {
        if (!head) {
            throw std::out_of_range("List is empty");
        }
        return head->data;
    }

    T at(int index) const {
        if (index < 0) {
            throw std::out_of_range("Invalid index");
        }

        std::shared_ptr<Node> curr = head;
        int i = 0;
        while (curr) {
            if (i == index) {
                return curr->data;
            }
            curr = curr->next;
            i++;
        }

        throw std::out_of_range("Index out of bounds");
    }

    [[nodiscard]] bool isEmpty() const {
        return head == nullptr;
    }

    [[nodiscard]] size_t size() const {
        size_t count = 0;
        for (auto link = head.get(); link != nullptr; ++count, link = link->next.get());
        return count;
    }

    void clear() {
        while (head) {
            head.swap(head->next);
        }
        head.reset();
    }
};