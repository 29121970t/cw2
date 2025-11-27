(function () {
    function initBridge(callback) {
        if (!window.QWebChannel || !window.qt) {
            callback(null);
            return;
        }
        new QWebChannel(qt.webChannelTransport, function (channel) {
            callback(channel.objects.bridge || null);
        });
    }

    document.addEventListener("DOMContentLoaded", function () {
        const body = document.body;
        const manual = document.querySelector(".manual-input");
        if (manual && body.dataset.pick !== "true") {
            manual.classList.add("hidden");
        }

        const latInput = document.getElementById("lat");
        const lonInput = document.getElementById("lon");
        if (latInput) {
            latInput.value = body.dataset.lat || "";
        }
        if (lonInput) {
            lonInput.value = body.dataset.lon || "";
        }

        const applyBtn = document.getElementById("applyCoords");
        if (!applyBtn) {
            return;
        }

        initBridge(function (bridge) {
            applyBtn.addEventListener("click", function () {
                if (!bridge) {
                    return;
                }
                const lat = parseFloat(latInput.value);
                const lon = parseFloat(lonInput.value);
                if (Number.isNaN(lat) || Number.isNaN(lon)) {
                    return;
                }
                bridge.pick(lat, lon);
            });
        });
    });
})();

