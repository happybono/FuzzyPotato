<script type="text/javascript">

// set your channel id here
var channel_id = '[ThingSpeak Channel ID]';
// set your channel's read api key here if necessary
var api_key = '[ThingSpeak Read API Key]';

// variable for the data point
var p_temp,
  p_humi,
  p_lux,
  p_smoist,
  p_ssalinity,
  p_battvolt,
  p_timestamp,
  timestamp;

function initPage() {
  loadData();
  setInterval(loadData, 15000);
}

function loadData() {
  p_temp = 0;
  p_humi = 0;
  p_lux = 0;
  p_smoist = 0;
  p_ssalinity = 0;
  p_battvolt = 0;
  p_timestamp = 0;

  // get the data from thingspeak
  $.getJSON('https://api.thingspeak.com/channels/' + channel_id + '/feed/last.json?results=1&api_key=' + api_key, function(data) {
    // get the data points
    p_temp = parseFloat(data.field1);
    p_humi = parseFloat(data.field2);
    p_lux = parseFloat(data.field3);
    p_smoist = parseFloat(data.field4);
    p_ssalinity = parseFloat(data.field5);
    p_battvolt = parseFloat(data.field6);
    p_timestamp = new Date(data.created_at);
    timestamp = p_timestamp.toLocaleString();
    outputData();
  });
}
  
  function outputData() {
  // update page
  document.getElementById('temperature').innerHTML = p_temp + ' °C';
  document.getElementById('humidity').innerHTML = p_humi + ' %';
  document.getElementById('lx').innerHTML = p_lux + ' ㏓';
  document.getElementById('smoisture').innerHTML = p_smoist + ' %';
  document.getElementById('ssalinity').innerHTML = p_ssalinity + ' %';
  //  document.getElementById('battvolt').innerHTML = p_battvolt + ' ㎷ / 4330.00 ㎷ (' + (100*(p_battvolt-2800)/(4330-2800)).toFixed(0) + ' %)';
  if (p_battvolt >= 3066) {
  document.getElementById('battvolt').innerHTML = p_battvolt + ' ㎷ / 4330.00 ㎷ (' + (100*(p_battvolt-2800)/(4330-2800)+5).toFixed(0) + ' %)';
  }
  if (p_battvolt < 3066) {
  document.getElementById('battvolt').innerHTML = p_battvolt + ' ㎷ / 4330.00 ㎷ (' + (100*(p_battvolt-2800)/(4330-2800)-2.5).toFixed(0) + ' %)'; 
  }
  document.getElementById('timestamp').innerHTML = '<i class="material-icons" style="font-size:1.08em;">update</i>' + " " + timestamp;
}

</script>
