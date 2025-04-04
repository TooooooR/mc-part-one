const socket = new WebSocket('ws://' + window.location.hostname + ':81');
const algo1 = document.getElementById("algo1");
const algo2 = document.getElementById("algo2");
const red = document.getElementById("red");
const yellow = document.getElementById("yellow");
const green = document.getElementById("green");

const ledElements = {
    0: red,
    1: yellow,
    2: green
};

function fetching() {
    fetch("/click")
        .then(response => response.text())
        .then(data => console.log(data));
};

function sendingUART() {
    fetch("/sendD")
        .then(response => response.text())
        .then(data => console.log(data));
}

socket.onmessage = function(event) {
    const data = JSON.parse(event.data);
    data.leds.forEach((state, index) => {
        ledElements[index].style.opacity = state ? "1" : "0.3";
    });
};

algo1.addEventListener("click", () => {
    fetching();
});

algo2.addEventListener("click", () => {
    sendingUART();
});
