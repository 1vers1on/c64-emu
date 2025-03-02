<script lang="ts">
    import { emulatorSettings  } from "$lib/stores/emulator";
    function virtualKeyboard() {
        console.log("Virtual keyboard activated");
    }

    function joystickMode() {
        console.log("Joystick mode activated");
    }

    function loadProgram() {
        console.log("Load program dialog");
    }

    function loadState() {
        emulatorSettings.update(settings => {
            // settings.saveState = !settings.saveState;
            settings.loadState = true;
            return settings;
        });
    }

    function saveState() {
        emulatorSettings.update(settings => {
            settings.saveState = true;
            return settings;
        });
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
        <button class="btn btn-secondary" onclick={virtualKeyboard}>
            <i class="fas fa-keyboard"></i> Virtual Keyboard
        </button>
        <button class="btn btn-secondary" onclick={joystickMode}>
            <i class="fas fa-gamepad"></i> Joystick Mode
        </button>
        <button class="btn btn-secondary" onclick={saveState}>
            <i class="fas fa-save"></i> Save Memory        
        </button>
        <!-- <button class="btn btn-secondary" onclick={loadState}>
            <i class="fas fa-folder-open"></i> Load State
        </button> -->
        <button class="btn btn-secondary" onclick={exportFile}>
            <i class="fas fa-download"></i> Export
        </button>
        <button class="btn btn-secondary" onclick={showHelp}>
            <i class="fas fa-question-circle"></i> Help
        </button>
    </div>
</div>
