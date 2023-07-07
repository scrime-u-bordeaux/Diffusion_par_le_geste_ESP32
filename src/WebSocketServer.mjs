
import { WebSocketServer } from "ws"; 

import {volumeValue, hpCorrespondingNumber, selectedHp, NBR_HP, lookForElementInArray} from "./relationMacAddressOscAddress.js";

import { Client, Message } from 'node-osc';

const client = new Client('192.168.1.2', 9997);

const server = new WebSocketServer ({port: 8080});


function printMessageReceived(data) { 
    data = data + ''; // split method doesn't work without this 
    const newData = data.split('\n');
    console.log("Received data from:",newData[0]); 
    for (let i = 1; i < newData.length; i++) {
      console.log(newData[i]);
    }
}

function receivingData(data) {
    data = data + ''; // split method doesn't work without this
    const newData = data.split('\n');
    //console.log(newData);
    if (newData[0] === "E8:DB:84:1F:31:54" ) { //the remote control macAddress 
        interprateRemoteControlData(newData);
    }
    const idx = hpCorrespondingNumber(newData[0]);
    if (idx !== -1) { //-1 means that the delivered macAddress isn't an esp related to a high speaker 
      interprateHpData(newData, idx);
    }
    else {
      printMessageReceived(data);
    }
}

//In perspective of adding new buttons on the remote, I made a generic function
function interprateRemoteControlData(data) {
    volumeButtonPressed(data);
}

function volumeButtonPressed(data) {
  const volume = data[2];
  //console.log(volume);
  const dt = data[4] / 1000; //to go from milliseconds to seconds 
  const dvolume = volume * dt; //getting value of the gyroscope 
  const pDeltaVolume = valAbs(dvolume / 0.5); // percentage of the value of the percentage
  const _ = (pDeltaVolume) * 74; 
  const rest = (volume < 0) ? - _ : _; // just to rectify because the absolute ablolute value is taken previously 
  for (let i = 0; i < NBR_HP; i++) {
    if (selectedHp[i]) {
      const oldOscValueSent = volumeValue[i];
      const newOscValue = (oldOscValueSent + rest < -70) ? -70 : (oldOscValueSent + rest > 4) ? 4 : oldOscValueSent + rest; //-70 & 4 are the range value in score
      const message = new Message("volume/"+`${i + 1}`);
      message.append(newOscValue);
      console.log(volumeValue);
      client.send(message,errorCallBack);
      volumeValue[i] = newOscValue;
    }
  }
}

function interprateHpData(data, idx) {
  console.log("Hp Received data");
  console.log(data);
  selectedHp[idx] = parseInt(data[2]);
  console.log(selectedHp);
}


function valAbs(a) {
  return (a < 0) ? -a : a; 
}


function connection(ws) { 
    ws.on("error", console.error);
    ws.on("message", receivingData);
}



server.on("connection",connection);



function errorCallBack(err) {
  if (err) {
    console.error(new Error(err));
  }

}





