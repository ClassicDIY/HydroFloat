const char app_script_js[] PROGMEM = R"rawliteral(
    try {
        const relayThresholds = [];
        const fields = document.querySelectorAll("#app_fields .fld");

        fields.forEach(fld  => {
            const threshold = parseInt(fld.querySelector("input[name='threshold']").value, 10);
            const textLabel = fld.querySelector("input[name='label']").value;

            relayThresholds.push({
            threshold: threshold,
            label: textLabel,
            active: true
            });
        });
        const payload = { relayThresholds };
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
