(function () {
    let bridge = null;
    let map = null;
    let marker = null;

    function initBridge() {
        if (!window.QWebChannel || !window.qt) {
            return;
        }
        new QWebChannel(qt.webChannelTransport, function (channel) {
            bridge = channel.objects.bridge || null;
        });
    }

    function showError(message) {
        const mapElement = document.getElementById("map");
        const errorElement = document.getElementById("error");
        if (!errorElement) {
            return;
        }
        if (mapElement) {
            mapElement.style.display = "none";
        }
        errorElement.classList.add("visible");

        let manualBlock = "";
        if (document.body.dataset.pick === "true") {
            const lat = document.body.dataset.lat || "";
            const lon = document.body.dataset.lon || "";
            manualBlock = `
                <div class="manual-input">
                    <div class="muted" style="margin-bottom:10px;">Введите координаты вручную:</div>
                    <div class="inputs">
                        <input id="lat" placeholder="Широта" type="number" step="0.000001" value="${lat}">
                        <input id="lon" placeholder="Долгота" type="number" step="0.000001" value="${lon}">
                        <button id="applyCoords">Применить</button>
                    </div>
                </div>`;
        }

        errorElement.innerHTML = `
            <div>
                <div class="message" style="font-size:15px;margin-bottom:8px;">${message}</div>
                ${manualBlock}
            </div>`;

        const applyBtn = document.getElementById("applyCoords");
        if (applyBtn) {
            applyBtn.addEventListener("click", function () {
                if (!bridge) {
                    return;
                }
                const latInput = document.getElementById("lat");
                const lonInput = document.getElementById("lon");
                const lat = parseFloat(latInput.value);
                const lon = parseFloat(lonInput.value);
                if (Number.isNaN(lat) || Number.isNaN(lon)) {
                    return;
                }
                bridge.pick(lat, lon);
            });
        }
    }

    function handleMapClick(event) {
        if (!marker || !bridge) {
            return;
        }
        marker.setPosition(event.latLng);
        bridge.pick(event.latLng.lat(), event.latLng.lng());
    }

    window.init = function () {
        const body = document.body;
        const lat = parseFloat(body.dataset.lat) || 0.0;
        const lon = parseFloat(body.dataset.lon) || 0.0;
        const pickMode = body.dataset.pick === "true";
        const center = { lat, lng: lon };

        try {
            map = new google.maps.Map(document.getElementById("map"), {
                zoom: 15,
                center,
                gestureHandling: "greedy",
                streetViewControl: false
            });
            marker = new google.maps.Marker({ position: center, map });

            if (pickMode) {
                map.addListener("click", handleMapClick);
            }
        } catch (err) {
            showError("Не удалось инициализировать Google Maps. " + err.message);
        }
    };

    window.gm_authFailure = function () {
        showError("Ошибка аутентификации Google Maps API: неверный или ограниченный ключ.");
    };

    initBridge();
})();

