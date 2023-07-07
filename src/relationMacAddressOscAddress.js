const NBR_HP = 2;
const NBR_RC = 1;//useless at the moment, but maybe needed when a second remote will be built

const addressMacHp = [ "E8:DB:84:1F:48:C8","E8:DB:84:1E:73:04"];
const addressMacRc = ["E8:DB:84:1F:31:54"]; //useless at the moment, but maybe needed when a second remote will be built 

let volumeValue = createElementArray([], -70, 0, NBR_HP);
const selectedHp = createElementArray([],0,0,NBR_HP);

//Create a 'length' lengthed array with the same 'element'. 
//'idx' and 'array' were think to be initialized at 0 and [].
function createElementArray(array, element, idx, length) {
    if (idx >= length) {
        return array;
    }
    else {
        return createElementArray([...array, element], element, idx + 1, length);
    }
}

//returns the index of 'element' in the 'array', -1 otherwise
function lookForElementInArray(element, array, length) {
    for (let k = 0; k < length; k++) {
        if (array[k] === element) {
            return k;
        }
    }
    return -1;
}

//Just to generalize the function
function hpCorrespondingNumber(address) {
    return lookForElementInArray(address, addressMacHp, NBR_HP);
}

export {
    volumeValue,
    lookForElementInArray,
    hpCorrespondingNumber,
    selectedHp,
    NBR_HP
};
