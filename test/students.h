#include <iostream>
#include <pqxx/pqxx>
#include <memory>
#include <map>
#include <mutex>
#include <thread>

struct Student {
    int id;
    std::string name;
    int age;
};

class StudentDatabase {
private:
    std::map<int, std::shared_ptr<Student>> students;
    std::mutex dbMutex;
    pqxx::connection connection;

public:
    StudentDatabase(const std::string& connectionString) : connection(connectionString) {}

    void addStudent(int id, const std::string& name, int age) {
        std::shared_ptr<Student> newStudent = std::make_shared<Student>(Student{id, name, age});

        std::lock_guard<std::mutex> lock(dbMutex);

        students[id] = newStudent;

        pqxx::work txn(connection);
        std::string query = "INSERT INTO students (id, name, age) VALUES (" +
                            std::to_string(id) + ", '" + name + "', " + std::to_string(age) + ")";
        txn.exec(query);
        txn.commit();
    }

    void removeStudent(int id) {
        std::lock_guard<std::mutex> lock(dbMutex);

        students.erase(id);

        pqxx::work txn(connection);
        std::string query = "DELETE FROM students WHERE id = " + std::to_string(id);
        txn.exec(query);
        txn.commit();
    }

    void getStudentInfo(int id) {
        std::lock_guard<std::mutex> lock(dbMutex); // Locking the mutex for database access

        auto it = students.find(id);
        if (it != students.end()) {
            std::cout << "Student Info - ID: " << it->second->id << ", Name: " << it->second->name << ", Age: " << it->second->age << std::endl;
        } else {
            std::cout << "Student with id " << id << " not found." << std::endl;
        }
    }
};