#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip> // Za postavljanje preciznosti izlaza
#include <cmath>   // Za funkciju abs() za izračunavanje apsolutne vrednosti
#include <vector>  // Za korišćenje vektora

int main() {
    std::ifstream file1("talas.surf");
    std::ifstream file2("talas1.surf");
    std::ofstream output("output.txt");
    std::ofstream diffOutput("diff_output.txt");
    std::ofstream errorOutput("error_output.txt");

    if (!file1.is_open() || !file2.is_open()) {
        std::cerr << "Ne mogu otvoriti fajlove za čitanje!" << std::endl;
        return 1;
    }

    if (!output.is_open() || !diffOutput.is_open() || !errorOutput.is_open()) {
        std::cerr << "Ne mogu otvoriti fajlove za pisanje!" << std::endl;
        return 1;
    }

    std::string line1, line2;
    double totalErrorPercentage = 0;
    double maxErrorPercentage = 0;
    int count = 0;
    int validCount = 0;
    std::vector<double> maxErrorsPerLine; // Vektor za praćenje najvećih grešaka po liniji

    while (getline(file1, line1) && getline(file2, line2)) {
        std::istringstream iss1(line1);
        std::istringstream iss2(line2);
        double num1, num2;
        double maxErrorPerLine = 0; // Najveća greška u tekućoj liniji

        while (iss1 >> num1 && iss2 >> num2) {
            count++;
            if (num1 == num2) {
                output << "." << " ";
                diffOutput << "0 " << " ";
                errorOutput << "0% " << " ";
            } else {
                output << num1 << "(" << num2 << ") ";
                diffOutput << std::abs(num1 - num2) << " ";
                if (num1 != 0) {
                    double errorPercentage = std::abs((num2 - num1)) /std::abs(num1) * 100;
                    totalErrorPercentage += errorPercentage;
                    validCount++;
                    errorOutput << std::fixed << std::setprecision(2) << errorPercentage << "% ";
                    if (errorPercentage > maxErrorPerLine) {
                        maxErrorPerLine = errorPercentage;
                    }
                    if (errorPercentage > maxErrorPercentage) {
                        maxErrorPercentage = errorPercentage;
                    }
                } else {
                    errorOutput << "N/A ";
                }
            }
        }
        output << "\n";
        diffOutput << "\n";
        errorOutput << "\n";
        maxErrorsPerLine.push_back(maxErrorPerLine); // Dodavanje najveće greške ove linije u vektor
    }

    file1.close();
    file2.close();
    output.close();
    
    diffOutput.close();
    errorOutput.close();

    // Ispisivanje ukupnog procenta greške i najvećeg procenta greške u terminalu
    if (validCount > 0) {
        double averageErrorPercentage = totalErrorPercentage / validCount;
        std::cout << "Ukupan procenat greške: " << std::fixed << std::setprecision(2) << averageErrorPercentage << "%" << std::endl;
        std::cout << "Najveći procenat greške: " << std::fixed << std::setprecision(2) << maxErrorPercentage << "%" << std::endl;
        
        // Ispisivanje najveće greške po liniji
        std::cout << "Najveći procenat greške po liniji:" << std::endl;
        for (size_t i = 0; i < maxErrorsPerLine.size(); ++i) {
            std::cout << "Linija " << i+1 << ": " << std::fixed << std::setprecision(2) << maxErrorsPerLine[i] << "%" << std::endl;
        }
    } else {
        std::cout << "Nema validnih izračunatih procenta greške." << std::endl;
    }

    return 0;
}
