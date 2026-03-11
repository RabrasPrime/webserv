#include <iostream>
#include <cctype>   // Pour std::toupper

int main() {
    const int TAILLE_TAMPON = 1024;
    char tampon[TAILLE_TAMPON];
    std::streamsize total_bytes = 0; // Notre compteur

    // std::cout << "Entrez du texte (Ctrl+D / Ctrl+Z pour finir) :\n" << std::endl;
	std::cerr << "MY TESTER" << std::endl;
	std::cout << "HEADER\r\n\r\n";

    // Boucle de lecture par blocs
    while (std::cin.read(tampon, TAILLE_TAMPON) || std::cin.gcount() > 0) {
        std::streamsize nb_lus = std::cin.gcount();
        total_bytes += nb_lus; // On incrémente le compteur

        // Transformation en majuscules
        for (int i = 0; i < nb_lus; ++i) {
            tampon[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(tampon[i])));
        }

        // Affichage du bloc transformé
        std::cout.write(tampon, nb_lus);
    }

    // Affichage du rapport final
    std::cerr << "\n\n--- Rapport ---" << std::endl;
    std::cerr << "Nombre total d'octets (bytes) envoyés : " << total_bytes << " bytes." << std::endl;

    return 0;
}