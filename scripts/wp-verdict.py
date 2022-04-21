import json
import sys

def main():
    if len(sys.argv) != 2:
        print ("Usage : python3 " + sys.argv[0] + " [RESULT].json")
    
    result = sys.argv[1]

    isValidResult = False

    with open(result) as file:
        data = json.load(file)
        gbl = data["wp:global"]
        mn = gbl["wp:main"]
        total = mn["total"]
        nb_valid = 0
        if "valid" in mn:
            nb_valid = mn["valid"]
        print("Validated : "+ str(nb_valid))
        print("Total : "+ str(total))

if __name__ == '__main__':
    sys.exit(main())

    



