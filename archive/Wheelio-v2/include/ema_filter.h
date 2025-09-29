#ifndef EMA_FILTER_H
#define EMA_FILTER_H

class EMAFilter {
private:
    float alpha; // Smoothing factor
    float emaValue; // Current EMA value
    bool initialized; // To check if the filter is initialized

public:
    // Constructor
    EMAFilter(float alpha) : alpha(alpha), emaValue(0), initialized(false) {}

    // Update the EMA with a new value
    float update(float newValue) {
        if (!initialized) {
            emaValue = newValue; // Initialize EMA with the first value
            initialized = true;
        } else {
            emaValue = alpha * newValue + (1 - alpha) * emaValue;
        }
        return emaValue;
    }

    // Get the current EMA value
    float getValue() const {
        return emaValue;
    }

    // Set a new alpha value
    void setAlpha(float newAlpha) {
        alpha = newAlpha;
    }
};

#endif // EMA_FILTER_H