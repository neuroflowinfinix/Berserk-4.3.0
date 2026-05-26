importScripts('berserk.js');
let engineInstance;
Berserk({
  print:    (text) => postMessage(text),
  printErr: (err)  => console.error("Berserk Error:", err)
}).then((instance) => {
  engineInstance = instance;
  postMessage("readyok");
});
onmessage = (e) => {
  if (engineInstance)
    engineInstance.ccall('ExecCommand', 'null', ['string'], [e.data]);
};
