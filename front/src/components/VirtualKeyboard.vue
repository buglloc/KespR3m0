<template>
  <div class="virtual-keyboard"></div>
</template>

<script lang="ts">
import Keyboard from "simple-keyboard";
import "simple-keyboard/build/css/index.css";

export default {
  name: "VirtualKeyboard",
  data: () => ({
    keyboard: null
  }),
  mounted() {
    this.keyboard = new Keyboard(".virtual-keyboard", {
      onChange: input => this.onChange(input),
      onKeyPress: button => this.onKeyPress(button),
      theme: "kesprTheme hg-theme-default",
      physicalKeyboardHighlight: true,
      mergeDisplay: true,
      disableButtonHold: true,
      tabCharOnTab: true,
      debug: true,
      layout: {
        default: [
          "{escape} 1 2 3 4 5 6 7 8 9 0 - = {backspace} {insert}",
          "{tab} q w e r t y u i o p [ ] {delete}",
          "{capslock} a s d f g h j k l ; ' \\ {enter} {pageup}",
          "{shiftleft} ` z x c v b n m , . / {arrowup} {pagedown}",
          "{fn} {controlleft} {altleft} {metaleft} {space} {fn} {arrowleft} {arrowdown} {arrowright}"
        ],
        shift: [
          "{escape} ! @ # $ % ^ & * ( ) _ + {backspace} {insert}",
          "{tab} Q W E R T Y U I O P { } {delete}",
          '{capslock} A S D F G H J K L : " | {enter} {pageup}',
          "{shiftleft} ~ Z X C V B N M < > ? {arrowup} {pagedown}",
          "{fn} {controlleft} {altleft} {metaleft} {space} {fn} {arrowleft} {arrowdown} {arrowright}"
        ],
        fn: [
          "{escape} {f1} {f2} {f3} {f4} {f5} {f6} {f7} {f8} {f9} {f10} {f11} {f12} {backspace} {insert}",
          "{tab} q w e r t y u i o p [ ] {delete}",
          "{capslock} a s d f g h j k l ; ' \\ {enter} {pageup}",
          "{shiftleft} ` z x c v b n m , . / {arrowup} {pagedown}",
          "{fn} {controlleft} {altleft} {metaleft} {space} {fn} {arrowleft} {arrowdown} {arrowright}"
        ],
      },
      display: {
        "{escape}": "Esc",
        "{tab}": "⇥",
        "{backspace}": "Backspace",
        "{enter}": "Enter ↵",
        "{capslock}": "Caps",
        "{shiftleft}": "Shift",
        "{shiftright}": "Shift",
        "{controlleft}": "Ctrl ⌃",
        "{controlright}": "Ctrl ⌃",
        "{altleft}": "Alt ⌥",
        "{altright}": "Alt ⌥",
        "{metaleft}": "Cmd ⌘",
        "{metaright}": "Cmd ⌘",
        "{insert}": "Ins",
        "{delete}": "Del",
        "{pageup}": "PgUp",
        "{pagedown}": "PgDn",
        "{fn}": "Fn",
        "{f1}": "F1",
        "{f2}": "F2",
        "{f3}": "F3",
        "{f4}": "F4",
        "{f5}": "F5",
        "{f6}": "F6",
        "{f7}": "F7",
        "{f8}": "F8",
        "{f9}": "F9",
        "{f10}": "F10",
        "{f11}": "F11",
        "{f12}": "F12",
      }
    });
  },
  methods: {
    onKeyPress(button: string) {
      this.$emit("onKeyPress", button);

      if (
        button === "{shift}" ||
        button === "{shiftleft}" ||
        button === "{shiftright}" ||
        button === "{capslock}"
      ) {
        this.toggleLayout("shift");
        return;
      }

      if (
        button === "{fn}"
      ) {
        this.toggleLayout("fn");
        return;
      }
    },
    toggleLayout(layout: string) {
      const currentLayout = this.keyboard.options.layoutName;
      let newLayout = "";
      switch (currentLayout) {
        case "default":
          newLayout = layout;
          break;
        case layout:
          newLayout = "default";
          break;
        default:
          return;
      }

      this.keyboard.setOptions({
        layoutName: newLayout
      });
    },
  },
  watch: {
    // input(input) {
    //   this.keyboard.setInput(input);
    // }
  }
};
</script>

<style scoped>
.virtual-keyboard.kesprTheme {
  color: rgb(18, 18, 18);
  border-radius: 0;
  border-bottom-right-radius: 1px;
  border-bottom-left-radius: 1px;
}

.virtual-keyboard.kesprTheme :deep(.hg-button) {
  height: 50px;
  display: flex;
  justify-content: center;
  align-items: center;
}

.virtual-keyboard.kesprTheme :deep(.hg-button.hg-functionBtn.hg-button-space) {
  width: 240px;
}

.virtual-keyboard.kesprTheme :deep(.hg-button:active) {
  background: rgb(218, 220, 228);
}

.virtual-keyboard.kesprTheme.hg-layout-shift :deep(.hg-button-capslock) {
  background: rgb(218, 220, 228);
}

.virtual-keyboard.kesprTheme.hg-layout-shift :deep(.hg-button-shiftleft) {
  background: rgb(218, 220, 228);
}

.virtual-keyboard.kesprTheme.hg-layout-fn :deep(.hg-button-fn) {
  background: rgb(218, 220, 228);
}
</style>
