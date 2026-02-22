const char onLoadScript[] PROGMEM = R"rawliteral(

)rawliteral";

const char app_script_base[] PROGMEM = R"rawliteral(
    const ModeEnum = {
        float: 0,
        pump: 1
    };

    function getModeNameFromValue(value) {
        return Object.keys(ModeEnum).find(key => ModeEnum[key] === value);
    }

)rawliteral";

const char app_script_select[] PROGMEM = R"rawliteral(
    if (el.name === "mode") {
        el.value = getModeNameFromValue(v);
    }
)rawliteral";

const char app_script_js[] PROGMEM = R"rawliteral(
try {
    const mode = document.querySelector("#mode").value;
    let payload = { 
        mode: ModeEnum[mode] 
    };

    const relayThresholds = [];

    const baseState = document.querySelector("input[name='base_label']").value;

    const fields = document.querySelectorAll("#levels .fld");
    const relayFields = Array.from(fields).slice(1); // skip base state

    relayFields.forEach(fld => {
        const threshold = parseInt(fld.querySelector("input[name='threshold']").value, 10);
        const textLabel = fld.querySelector("input[name='label']").value;

        relayThresholds.push({
            threshold,
            label: textLabel,
            active: true
        });
    });

    payload.baseState = baseState;
    payload.relayThresholds = relayThresholds;
    // Read the calibration offset
    const offsetInput = document.getElementById("_calibrationOffset");
    payload._calibrationOffset = parseFloat(offsetInput.value);
    // Read the range limits
    const lowerLimit = document.getElementById("_lowerLimit");
    payload._lowerLimit = parseFloat(lowerLimit.value);
    const upperLimit = document.getElementById("_upperLimit");
    payload._upperLimit = parseFloat(upperLimit.value);


    console.log(payload);

    const res = await fetch('/app_fields', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
    });

    console.log(await res.text());
    if (!res.ok) throw new Error('Save failed');

    status.innerHTML = '<span class="ok">APP Settings saved.</span>';

} catch (e) {
    status.innerHTML = '<span class="err">' + e.message + '</span>';
}
)rawliteral";


const char app_validateInputs[] PROGMEM = R"rawliteral(

	const _lowerLimit = parseFloat(document.getElementById("_lowerLimit").value);
	const _upperLimit = parseFloat(document.getElementById("_upperLimit").value);
	if (_lowerLimit >= _upperLimit) {
		alert('The lower limit must be lower than upper limit!');
		return false;
	}
)rawliteral";