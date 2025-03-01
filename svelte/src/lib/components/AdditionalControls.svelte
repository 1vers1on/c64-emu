<script>
    function virtualKeyboard() {
        console.log("Virtual keyboard activated");
    }

    function joystickMode() {
        console.log("Joystick mode activated");
    }

    function loadProgram() {
        console.log("Load program dialog");
    }

    function saveState() {
        console.log("Save state");
    }

    function exportFile() {
        console.log("Export file");
    }

    function showHelp() {
        console.log("Help dialog");
    }
</script>

<div class="glass-panel" style="margin-top: 1.5rem;">
    <div class="panel-header">
        <div class="panel-title">Additional Controls</div>
        <div>
            <i class="fas fa-cog"></i>
        </div>
    </div>

    <div class="controls">
        <button class="btn btn-secondary" on:click={virtualKeyboard}>
            <i class="fas fa-keyboard"></i> Virtual Keyboard
        </button>
        <button class="btn btn-secondary" on:click={joystickMode}>
            <i class="fas fa-gamepad"></i> Joystick Mode
        </button>
        <button class="btn btn-secondary" on:click={loadProgram}>
            <i class="fas fa-folder-open"></i> Load Program
        </button>
        <button class="btn btn-secondary" on:click={saveState}>
            <i class="fas fa-save"></i> Save State
        </button>
        <button class="btn btn-secondary" on:click={exportFile}>
            <i class="fas fa-download"></i> Export
        </button>
        <button class="btn btn-secondary" on:click={showHelp}>
            <i class="fas fa-question-circle"></i> Help
        </button>
    </div>
</div>
