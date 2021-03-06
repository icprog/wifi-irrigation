//

var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);
connection.onopen = function () {
  connection.send('Connect ' + new Date());
  connection.send("?STATE");
  buttons_disabled(false);
};
connection.onerror = function (error) {
  console.log('WebSocket Error ', error);
};
connection.onmessage = function (e) {
  console.log('Server: ', e.data);
  var message = e.data.split(':');
  if(message[0] == "SOL"){
  	var num = Number(message[1]);
  	var state = Number(message[2]);
  	var button = document.getElementById(`sol_button_${num}`);
  	if(state){ button.style.backgroundColor = "#00E640"; }
  	else{ button.style.backgroundColor = "#CF000F"; }
  }
  else if("TIME" == message[0]){
  	var hour = Number(message[2]);
  	var minute = Number(message[3]);
  	var second = Number(message[4]);
  	var d = new Date(0, 0, 0, hour, minute, second);
    if ("UTC" == message[1]){
      var id = "utc_text";  
    }
    else{
      var id = "local_text";
    }
  	document.getElementById(id).innerHTML = d.toLocaleTimeString();
  }
};
connection.onclose = function () {
  console.log('WebSocket connection closed');
};

function toggle (num) {
	console.log(`Toggling solenoid ${num}`)
    connection.send(`SOL:${num}`);
}

function buttons_disabled(state){
	console.log(`Buttons disabled: ${state}`)
	var buttons = document.getElementsByClassName("sol");
	var i;
	for (i = 0; i < buttons.length; i++){
		buttons[i].disabled = state;
	}
}

function send_on_time(){
  var start_time = document.getElementById("start_time_setter").value;
  var run_time = document.getElementById("run_time_setter").value;
  var individuals = document.getElementsByName("v_enable_individual");
  console.log(`Sending start time: ${start_time}`);
  for(var i = 0; i < individuals.length; i++){
    console.log(individuals[i].value, individuals[i].checked);  
  }
  connection.send(`TIME:${start_time}`);
  connection.send(`RUN:${run_time}`);
}

function toggle_state(source){
  var individuals = document.getElementsByName("v_enable_individual");
  for(var i = 0; i < individuals.length; i++){
    individuals[i].checked = source.checked;  
  }
}