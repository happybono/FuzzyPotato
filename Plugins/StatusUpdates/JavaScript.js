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

// Object to store previous values for per-character comparison.
var previousValues = {
  temperature: '',
  humidity: '',
  lx: '',
  smoisture: '',
  ssalinity: '',
  battvolt: '',
  timestamp_text: ''   // Used to preserve the fixed icon while animating only the timestamp text
};

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
  $.getJSON(
    'https://api.thingspeak.com/channels/' + channel_id + '/feed/last.json?results=1&api_key=' + api_key,
    function(data) {
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
    }
  );
}

/* Helper to check if a character is a digit */
function isDigit(ch) {
  return /\d/.test(ch);
}

/* Animate a single digit from old to new.
   The animation updates the content of 'span' gradually, incrementing or decrementing by 1 every 50ms.
*/
function animateDigit(span, start, end) {
  var current = start;
  var step = (end > start) ? 1 : -1;
  var delay = 50; // milliseconds

  function update() {
    current += step;

    // If we haven't reached the target yet:
    if ((step > 0 && current < end) || (step < 0 && current > end)) {
      span.innerHTML = current;
      setTimeout(update, delay);
    } else {
      // Ensure final value is set and remove the flipping class.
      span.innerHTML = end;
      span.classList.remove("flipping");
    }
  }

  // Start only if the numbers differ.
  if (start !== end) {
    setTimeout(update, delay);
  }
}

/* Update an element's content with per-character flip animation.
   - Splits the new value into characters.
   - Generates a span for each character with a unique id so we can update it later.
   - Supports fixed prefix HTML (e.g., a static icon) while animating only the text part.
*/
function updateValue(elementId, newValue, keyOverride, prefixHtml) {
  var element = document.getElementById(elementId);
  var key = keyOverride || elementId;

  var previousText = previousValues[key] || '';
  var previousChars = previousText.split('');
  var newChars = String(newValue).split('');
  var html = '';

  for (var i = 0; i < newChars.length; i++) {
    var char = newChars[i];
    var needsFlip = (previousChars[i] === undefined || char !== previousChars[i]);
    var displayChar = (char === " ") ? "&nbsp;" : char;

    // Assign a unique id for each character span.
    html += '<span id="' + key + '_span_' + i + '" class="' + (needsFlip ? 'flipping' : '') + '">' +
            displayChar +
            '</span>';
  }

  // Render fixed prefix (if any) + animated spans
  element.innerHTML = (prefixHtml ? prefixHtml : '') + html;

  // For each character index that represents a digit and that needs updating,
  // run the counting animation if applicable.
  for (var j = 0; j < newChars.length; j++) {
    if (previousChars[j] !== undefined &&
        isDigit(previousChars[j]) &&
        isDigit(newChars[j]) &&
        previousChars[j] !== newChars[j]) {

      var spanElement = document.getElementById(key + '_span_' + j);
      animateDigit(spanElement, parseInt(previousChars[j], 10), parseInt(newChars[j], 10));
    }
  }

  previousValues[key] = String(newValue);
}

function outputData() {
  // update page (with per-character flip animation)
  document.getElementById('temperature'); // keeps original element id usage
  updateValue('temperature', p_temp + ' °C');

  document.getElementById('humidity');
  updateValue('humidity', p_humi + ' %');

  document.getElementById('lx');
  updateValue('lx', p_lux + ' ㏓');

  document.getElementById('smoisture');
  updateValue('smoisture', p_smoist + ' %');

  document.getElementById('ssalinity');
  updateValue('ssalinity', p_ssalinity + ' %');

  document.getElementById('battvolt');
  updateValue(
    'battvolt',
    (p_battvolt - 3250).toFixed(2) + ' ㎷ / ' +
    (4200 - 3250) + ' ㎷ (' +
    (100 * (p_battvolt - 3250) / (4200 - 3250)).toFixed(0) + ' %)'
  );

  // Update timestamp so that the fixed icon remains intact while the text animates
  updateValue(
    'timestamp',
    timestamp,
    'timestamp_text',
    '<i class="material-symbols-rounded" style="font-size:1.17em;">potted_plant</i> '
  );
}

// Expose initPage globally.
window.initPage = initPage;

</script>
