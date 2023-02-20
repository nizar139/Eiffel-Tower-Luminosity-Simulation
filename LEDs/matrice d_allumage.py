import argparse
import numpy as np
# from LEDs_Display import display
# Description des variables
# N_leds : nombre de LEDs à commander
# T_tot : temps total de scintillement (en nombre de tic)
# T_all : période d'allumage d'une LED
# P : proportion de LEDs allumées en même temps
# Création du parser pour gérer les variables simplements
parser = argparse.ArgumentParser(
    description='Choix des variables pour contrôler le scintillement')
parser.add_argument('-t', '--test', required=False,
                    action='store_true', help="Lancer le test")
parser.add_argument('-r', '--run', required=False, action='store_true',
                    help="Run le programme pour obtenir l'ouput final")
parser.add_argument('-v', '--values', type=float,
                    nargs=4, required=False, help='Choix des quatre valeurs (N_leds, T_tot, T_all, P)')
parser.add_argument('-rv', '--runvalues', type=float, nargs=6, required=False,
                    help='Choix des six valeurs (N_leds, T_tot, T_all, P, tic_coef, Port)')
# parser.add_argument('-d', '--display', required = False, action = 'store_true', help = "Afficher le rendu")
args = parser.parse_args()

# Fonction de scintillement


def scintillement(N_leds, T_tot, T_all, P):
    """ Renvoie un tableau de dimensions (T_tot, N_leds) représentant l'allumage des différentes LEDs au cours de la période """
    # Matrice de l'état des LEDs à chaque tic -- Un entier strictement positif correspond à une LED allumée, sinon elle est éteinte
    mat = np.empty((T_tot, N_leds), dtype=int)
    mat[0] = np.random.choice([k for k in range(
        T_all+1)], size=(N_leds,), p=[1-P] + [(P/T_all) for k in range(T_all)])
    # A chaque tic, on ajoute -1 à l'état de chacune des LEDs pour qu'une LED ne reste allumé que T_all tics d'affilés
    vect_maj = np.full((N_leds,), -1)
    for t in range(1, T_tot):
        mat[t] = mat[t-1]
        for i in range(N_leds):
            # Parmis les LEDs éteintes au précédent tour, certaines vont être allumées
            if mat[t][i] <= 0:
                # Proportion de LEDs éteintes au précédent tour qui vont être allumées
                coef = P/(T_all*(1-P))
                test = np.random.choice([0, T_all+1], p=[1-coef, coef])
                mat[t][i] = test
        mat[t] += vect_maj
    # On transforme maintenant cette matrice en matrice de 0 et 1
    array1 = np.where(mat <= 0, 0, mat)
    array2 = np.where(array1 > 0, 1, array1)
    return array2


def transform_into_C_matrix(Mat):
    """ Renvoie le tableau sous forme d'une matrice de 0 et de 1 en C """
    output = "{"
    for i in range(len(Mat)):
        for j in range(len(Mat[0])):
            output += f"{Mat[i][j]},"
    output = output[:-1]  # On retire la dernière virgule
    output += "}"
    output += ";"
    return output


def transform_into_minimal_C_matrix(Mat):
    """ Renvoie le tableau sous forme d'une matrice en C où chaque octet de la matrice original est réduit en un nombre entre 0 et 255 """
    output = "{"
    for i in range(len(Mat)):
        for j in range((len(Mat[0])//8)+1):
            octet = str()
            for k in range(8):
                if 8*j+k < len(Mat[0]):
                    octet += str(Mat[i][8*j+k])
                else:
                    octet += '0'
            output += f"{int(octet, 2)},"
    output = output[:-1]  # On retire la dernière virgule
    output += "}"
    output += ";"
    return output
# Création d'un output à copier dans Mbed


def final_output(parameters):
    """ Renvoie un ensemnle de strings qu'il suffira de copier dans le code en C """
    # parameters = (N_leds, T_tot, T_all, P, tic_coef, Port)
    output = str()
    output += f"PololuLedStrip ledStrip(D{parameters[5]}); \n"
    output += "\n"
    output += "// Nombre de LEDs (nécessairement un multiple de 8 suppérieur au nombre de LEDs réel) \n"
    output += f"#define LED_COUNT {parameters[0]} \n"
    output += "\n"
    output += "// Tic coefficient : \n"
    output += f"int tic_coef = {parameters[4]}; \n"
    output += "\n"
    output += "// Période globale : \n"
    output += f"int T_tot = {parameters[1]}; \n"
    output += "\n"
    output += "// Matrice d'allumage : \n"
    output += f"char Matrice_LEDs[{parameters[0]}][{parameters[1]}] = "
    return output
    # Gestion des erreurs


def get_values_from_parser_argument():
    """ Renvoie les valeurs récupérer dans le parser values """
    values = args.values
    N_leds, T_tot, T_all, P = values[0], values[1],
    values[2], values[3]
    if int(N_leds) != N_leds:
        raise TypeError('N_leds must be an integer')
    elif N_leds < 0:
        raise ValueError('N_leds must be positive')
    if int(T_tot) != T_tot:
        raise TypeError('T_tot must be an integer')
    elif T_tot < 0:
        raise ValueError('T_tot must be positive')
    if int(T_all) != T_all:
        raise TypeError('T_all must be an integer')
    elif T_all < 0:
        raise ValueError('T_all must be positive')
    if type(P) is not float:
        raise TypeError('P must be a float')
    elif P < 0 or P > 1:
        raise ValueError('P must be between 0 and 1')
    return int(N_leds), int(T_tot), int(T_all), P


def get_runvalues_from_parser_argument():
    """ Renvoie les valeurs récupérer dans le parser values """
    runvalues = args.runvalues
    N_leds, T_tot, T_all, P, tic_coef, Port = runvalues[0], runvalues[
        1], runvalues[2], runvalues[3], runvalues[4], runvalues[5]
    if int(N_leds) != N_leds:
        raise TypeError('N_leds must be an integer')
    elif N_leds < 0:
        raise ValueError('N_leds must be positive')
    if int(T_tot) != T_tot:
        raise TypeError('T_tot must be an integer')
    elif T_tot < 0:
        raise ValueError('T_tot must be positive')
    if int(T_all) != T_all:
        raise TypeError('T_all must be an integer')
    elif T_all < 0:
        raise ValueError('T_all must be positive')
    if type(P) is not float:
        raise TypeError('P must be a float')
    elif P < 0 or P > 1:
        raise ValueError('P must be a value between 0 and 1')
    if int(tic_coef) != tic_coef:
        raise TypeError('tic_coef must be an integer')
    elif tic_coef <= 0:
        raise ValueError('tic_coef must be strictly positive')
    if int(Port) != Port:
        raise TypeError('Port must be an integer')
    if Port <= 0 or Port > 12:
        raise ValueError('Port must be a value between 1 and 12')
    return int(N_leds), int(T_tot), int(T_all), P, int(tic_coef), int(Port)

    # Programme principal
if __name__ == "__main__":
    if args.test:
        try:
            if args.values != None:
                N_leds, T_tot, T_all, P = get_values_from_parser_argument()
            else:
                N_leds, T_tot, T_all, P = 30, 100, 3, 0.5
        except Exception as e:
            print("An error as occured in the input of the value :", e)
            print("Therefore, predefined values are used")
            N_leds, T_tot, T_all, P = 30, 100, 3, 0.5
        finally:
            print("Résultat du test du scintillement : ",
                  scintillement(N_leds, T_tot, T_all, P))
    if args.run:
        try:
            if args.runvalues != None:
                N_leds, T_tot, T_all, P, tic_coef, Port = get_runvalues_from_parser_argument()
            else:
                N_leds, T_tot, T_all, P, tic_coef, Port = 150, 200, 3, 0.3, 2, 8
        except Exception as e:
            print("An error as occured at the input of the runvalues :", e)
            print("Therefore, predefined values are used")
            N_leds, T_tot, T_all, P, tic_coef, Port = 150, 200, 3, 0.3, 2, 8
        finally:
            print(final_output((N_leds, T_tot, T_all, P, tic_coef, Port)) +
                  transform_into_minimal_C_matrix(scintillement(N_leds, T_tot, T_all, P)))
