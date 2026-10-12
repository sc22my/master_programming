//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5812M Foundations of Modelling & Rendering
//  User Interface for Coursework
//
// Quick sort algorithm
//  
///////////////////////////////////////////////////

// include guard
#ifndef SORT_H_
#define SORT_H_

#include <vector>

// Static class that contains quicksort implementation
class QuickSort {
public:
    // Sorts a list in place
    // Additional vectors also get sorted based on the values vector
    template<typename T, typename ValueFn, typename... AdditionalVectors>
    static void Sort(std::vector<T>& values, ValueFn value, std::vector<AdditionalVectors>&... additionalvecs) {
        // Call implementation fn
        Sort_impl(values, 0, values.size(), value, std::forward<std::vector<AdditionalVectors>&>(additionalvecs)...);
    }
private:
    // recursive implementation of quicksort
    template<typename T, typename ValueFn, typename... AdditionalVectors>
    static void Sort_impl(std::vector<T>& values, int l, int r, ValueFn value, std::vector<AdditionalVectors>&... additionalvecs) {
        // Stop condition
        if (r - l <= 1)
            return;

        // Select a pivot point and move it to the end
        unsigned int pivot = (l + r) / 2;
        std::swap(values[r-1], values[pivot]);
        (std::swap(additionalvecs[r-1], additionalvecs[pivot]), ...);

        // sort left and right
        int li = l;
        int ri = r - 2;

        while (li <= ri) {
            while (li <= ri && value(values[li]) <= value(values[r-1]))
                li++;
            while (li <= ri && value(values[ri]) >= value(values[r-1]))
                ri--;

            // Swap values
            if (li < ri) {
                std::swap(values[li], values[ri]);

                // Use fold expression to swap the additional vectors as well
                (std::swap(additionalvecs[li], additionalvecs[ri]), ...);
            }
        }

        // swap pivot to correct position
        std::swap(values[r-1], values[li]);
        (std::swap(additionalvecs[r-1], additionalvecs[li]), ...);

        // Recuse
        Sort_impl(values, l, li, value, std::forward<std::vector<AdditionalVectors>&>(additionalvecs)...);
        Sort_impl(values, li + 1, r, value, std::forward<std::vector<AdditionalVectors>&>(additionalvecs)...);
    }
};

#endif