(function (root) {
    function initBridge(callback) {
        const transport = root.qt?.webChannelTransport;
        if (!root.QWebChannel || !transport) {
            callback(null);
            return;
        }
        const channel = new root.QWebChannel(transport, function (instance) {
            callback(instance.objects.bridge || null);
        });
        return channel;
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
                const lat = Number.parseFloat(latInput.value);
                const lon = Number.parseFloat(lonInput.value);
                if (Number.isNaN(lat) || Number.isNaN(lon)) {
                    return;
                }
                bridge.pick(lat, lon);
            });
        });
    });
})(globalThis);

