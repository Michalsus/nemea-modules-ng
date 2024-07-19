/**
 * @file
 * @author Michal Matejka <xmatejm00@stud.fit.vutbr.cz>
 * @brief Circular Buffer
 *
 * Cirular buffer class for scan_detector module
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <unirec++/unirec.hpp>
#include <string>
#include <memory>
#include <optional>
#include <iostream>

using namespace Nemea;

class CircularBuffer {
public:
    CircularBuffer(int n) : buffer(std::make_unique<Line[]>(n)), head(nullptr), tail(nullptr), count(0), maxlines(n) {
        // Initialize circular references
        for (int i = 0; i < n; ++i) {
            buffer[i].next = &buffer[(i + 1) % n];
        }
        head = &buffer[0];
        tail = &buffer[0];
    }

    std::optional<UnirecRecord> buffInsert(const UnirecRecord unirecRecord) {
        if (count == maxlines) {
            // Buffer is full, overwrite the oldest element
            UnirecRecord tmp = head->unirecRecord;
            head->unirecRecord = unirecRecord;
            head = head->next;
            tail = tail->next;
            return tmp;
        } else {
            // Buffer is not full, add a new element
            tail->unirecRecord = unirecRecord;
            tail = tail->next;
            ++count;
            return std::nullopt;
        }
    }

    int size() const {
        return count;
    }

private:
    struct Line {
        Line* next;
        UnirecRecord unirecRecord;
    };

    std::unique_ptr<Line[]> buffer; // Array of Line structures
    Line* head; // Pointer to the oldest element
    Line* tail; // Pointer to the newest element
    int count;  // Current count of elements
    int maxlines; // Maximum number of elements
};
