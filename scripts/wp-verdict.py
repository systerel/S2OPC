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
        if "valid" in mn and mn["valid"] == total:
            isValidResult = True
    
    if isValidResult:
        print ("Valid")
    else:
        print ("Invalid")

if __name__ == '__main__':
    sys.exit(main())

    



