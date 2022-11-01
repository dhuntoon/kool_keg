import numpy as np
import sys

def main():
    # generate the table entry
    kegID = "1"
    
    #if an ID is provided, overwrite the default
    if len(sys.argv) > 1:
        kegID = sys.argv[1]

    out = generateTableEntry(kegID)
    
    # write the output to out.txt 
    f = open("out.txt", "w")
    f.write(out)
    f.close()

# Generate full keg entry
#
# @param kegID, the keg number
# @return json, the formatted string for the keg
def generateTableEntry(kegID):
    
    # Generate the random data set of the for [{"data":data,"kegId":1}] 
    data = generateRandomData()

    # build the string for the keg
    json = "{\"data\":" + data + ",\"kegId\":\"" + kegID + "\"}"

    return json


# Generate a data set with a random number of entries
#
# @return data, a JSON string of the data set
def generateRandomData():
    # the number of entries is random from the range [1,20]
    numEntries = np.random.randint(1,20)
    
    # build the string for the data set
    data = "["

    # for each data entry, generate a random data set of the form [entry1,entry2]
    for entry in range(numEntries):
        entryJSON = generateRandomEntry(entry+1)
        data += entryJSON
        if entry+1 < numEntries:
            data += ","
    
    # finish the data entry
    data += "]"

    return data 

# Generate an entry with random temperature and volume values
#   
# @param entry, the entry number
# @return entryID, a JSON string of the entry
def generateRandomEntry(entryID):
    
    # the entry number is the ID
    id = entryID

    # the random temp is from the range [30,50]
    temp = np.random.randint(30,50)

    # the ranfom vol is from the range [0,1000]
    vol = np.random.randint(0,1000)

    # build the string of the form {"temp":"30","vol":"500","id":"1"}
    entry = "{\"temp\":\"" + str(temp) + "\",\"vol\":\"" + str(vol) + "\",\"id\":\"" + str(id) + "\"}"

    # return the formatted string
    return entry



if __name__ == "__main__":
    main()
