const socket = io();

const portSelect = document.getElementById("port-select");
const connectBtn = document.getElementById("connect-btn");
const statusSpan = document.getElementById("status");
const buttons = document.querySelectorAll(".control-btn");
const sliders = document.querySelectorAll(".slider");
const toggles = document.querySelectorAll(".toggle-switch input");

// Throttling function to prevent flooding Serial
function throttle(func, limit) {
  let inThrottle;
  return function () {
    const args = arguments;
    const context = this;
    if (!inThrottle) {
      func.apply(context, args);
      inThrottle = true;
      setTimeout(() => (inThrottle = false), limit);
    }
  };
}

// Receive list of ports
socket.on("ports-list", (ports) => {
  portSelect.innerHTML =
    '<option value="" disabled selected>Select COM Port</option>';
  ports.forEach((port) => {
    const option = document.createElement("option");
    option.value = port.path;
    option.textContent = `${port.path} ${port.manufacturer || ""}`;
    portSelect.appendChild(option);
  });
});

// Handle connection status
socket.on("status", (msg) => {
  statusSpan.textContent = msg;
  if (msg.includes("Connected")) {
    statusSpan.className = "status connected";
    connectBtn.textContent = "Connected";
    connectBtn.disabled = true;
    portSelect.disabled = true;
  } else {
    statusSpan.className = "status disconnected";
    connectBtn.textContent = "Connect";
    connectBtn.disabled = false;
    portSelect.disabled = false;
  }
});

// Connect button click
connectBtn.addEventListener("click", () => {
  const selectedPort = portSelect.value;
  if (selectedPort) {
    statusSpan.textContent = "Connecting...";
    socket.emit("connect-port", selectedPort);
  } else {
    alert("Please select a COM port");
  }
});

// Handle Toggle Switches (Enable/Disable)
toggles.forEach((toggle) => {
  toggle.addEventListener("change", (e) => {
    const cmdOn = toggle.getAttribute("data-cmd-on");
    const cmdOff = toggle.getAttribute("data-cmd-off");
    const sliderId = toggle.id.replace("toggle-", "slider-");
    const slider = document.getElementById(sliderId);

    if (toggle.checked) {
      console.log("Sending:", cmdOn);
      socket.emit("command", cmdOn);
      if (slider) slider.disabled = false;
    } else {
      console.log("Sending:", cmdOff);
      socket.emit("command", cmdOff);
      if (slider) slider.disabled = true;
    }
  });
});

// Handle Sliders with Throttling
sliders.forEach((slider) => {
  const valDisplay = document.getElementById(
    slider.id.replace("slider-", "val-")
  );
  const prefix = slider.getAttribute("data-cmd-prefix");

  const sendCommand = throttle((val) => {
    const cmd = `${prefix}${val}`;
    console.log("Sending:", cmd);
    socket.emit("command", cmd);
  }, 50); // 50ms throttle

  slider.addEventListener("input", (e) => {
    const val = e.target.value;
    if (valDisplay) valDisplay.textContent = val;
    if (!slider.disabled) {
      sendCommand(val);
    }
  });
});

// Handle Buttons (Gripper Open/Close)
buttons.forEach((btn) => {
  const handleCommand = (e) => {
    e.preventDefault();
    const cmd = btn.getAttribute("data-cmd");
    if (cmd) {
      // Visual feedback
      btn.style.borderColor = "var(--accent-color)";
      btn.style.transform = "scale(0.95)";
      setTimeout(() => {
        btn.style.borderColor = "rgba(255, 255, 255, 0.1)";
        btn.style.transform = "scale(1)";
      }, 200);

      console.log("Sending:", cmd);
      socket.emit("command", cmd);
    }
  };

  btn.addEventListener("click", handleCommand);
  btn.addEventListener("touchstart", handleCommand, { passive: false });
});

/* ---------- Sequence Logic ---------- */
let sequence = [];
const sequenceList = document.getElementById("sequence-list");
const btnSave = document.getElementById("btn-save-pos");
const btnPlay = document.getElementById("btn-play-seq");
const btnClear = document.getElementById("btn-clear-seq");

function updateSequenceUI() {
  sequenceList.innerHTML = "";
  if (sequence.length === 0) {
    sequenceList.innerHTML = '<div class="empty-state">No steps recorded</div>';
    return;
  }

  sequence.forEach((step, index) => {
    const div = document.createElement("div");
    div.className = "sequence-item";
    div.innerHTML = `<span>Step ${index + 1}</span> <span>${
      step.length
    } cmds</span>`;
    sequenceList.appendChild(div);
  });
}

btnSave.addEventListener("click", () => {
  const step = [];
  // Capture state of all ENABLED sliders
  sliders.forEach((slider) => {
    if (!slider.disabled) {
      const prefix = slider.getAttribute("data-cmd-prefix");
      const val = slider.value;
      step.push(`${prefix}${val}`);
    }
  });

  if (step.length > 0) {
    sequence.push(step);
    updateSequenceUI();

    // Visual feedback
    const originalText = btnSave.textContent;
    btnSave.textContent = "Saved!";
    setTimeout(() => (btnSave.textContent = originalText), 500);
  } else {
    alert("Enable at least one servo to save a position.");
  }
});

btnPlay.addEventListener("click", async () => {
  if (sequence.length === 0) return;

  btnPlay.disabled = true;
  btnPlay.textContent = "Playing...";

  for (const step of sequence) {
    // Send all commands for this step
    step.forEach((cmd) => {
      console.log("Replay:", cmd);
      socket.emit("command", cmd);
    });

    // Wait 1.5s before next step (allow arm to move)
    await new Promise((r) => setTimeout(r, 1500));
  }

  btnPlay.disabled = false;
  btnPlay.textContent = "Play Sequence";
});

btnClear.addEventListener("click", () => {
  sequence = [];
  updateSequenceUI();
});
