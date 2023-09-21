#include <iostream>
#include <iomanip>
#include <vector>
#include <curl/curl.h>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

// Struct to hold the response data from the API
struct ResponseData {
    std::string baseCurrency;
    std::vector<std::string> currencies;
    std::vector<double> exchangeRates;
};

// Callback function to receive the response from the API
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    response->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to fetch exchange rates from the API
bool fetchExchangeRates(const std::string& baseCurrency, ResponseData& responseData) {
    std::string url = "https://v6.exchangerate-api.com/v6/984ca0a0495401f2a5546fdb/latest/" + baseCurrency;
    std::string response;

    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            json jsonData = json::parse(response);

            if (jsonData["result"] == "success") {
                responseData.baseCurrency = jsonData["base_code"].get<std::string>();

                json rates = jsonData["conversion_rates"];
                for (auto it = rates.begin(); it != rates.end(); ++it) {
                    responseData.currencies.push_back(it.key());
                    responseData.exchangeRates.push_back(it.value().get<double>());
                }

                curl_easy_cleanup(curl);
                return true;
            }
        }

        curl_easy_cleanup(curl);
    }

    return false;
}

int main() {
    // Prompt user for input
    std::cout << "Enter the base currency code: ";
    std::string baseCurrency;
    std::cin >> baseCurrency;

    // Fetch exchange rates from the API
    ResponseData responseData;
    if (!fetchExchangeRates(baseCurrency, responseData)) {
        std::cout << "Failed to fetch exchange rates from the API." << std::endl;
        return 1;
    }

    // Display available currencies
    std::cout << "Available currencies:" << std::endl;
    for (const std::string& currency : responseData.currencies) {
        std::cout << "- " << currency << std::endl;
    }

    // Prompt user for input
    std::cout << "\nEnter the amount: ";
    double amount;
    std::cin >> amount;

    std::cout << "Enter the target currency: ";
    std::string targetCurrency;
    std::cin >> targetCurrency;

    // Convert currency
    double exchangeRate = 0.0;
    for (size_t i = 0; i < responseData.currencies.size(); ++i) {
        if (responseData.currencies[i] == targetCurrency) {
            exchangeRate = responseData.exchangeRates[i];
            break;
        }
    }

    if (exchangeRate != 0.0) {
        double convertedAmount = amount * exchangeRate;
        std::cout << "\nConverted amount: " << std::fixed << std::setprecision(2) << convertedAmount << " " << targetCurrency << std::endl;
    } else {
        std::cout << "\nInvalid target currency." << std::endl;
    }

    return 0;
}
