<template>
  <div>
    <StatusBar
      :icons="[states.status]"
      title="UART Logger"
    />
  </div>
  <div>
    <div class="pa-2">
      <v-sheet class="bg-grey-darken-4 pa-4">
        <UartLog
          :height="logH"
          ref="logEl"
        />
      </v-sheet>
    </div>
    <v-footer>
      <UartInput
        class="w-100"
        :on-message="sendMessage"
      />
    </v-footer>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, watch } from 'vue';
import { useWindowSize, } from '@vueuse/core';
import { mdiConnection } from "@mdi/js";
import { storeToRefs } from "pinia";
import { useR3m0teStore } from "../stores/remote";
import { Msg, MsgUartRx } from "../types";
import UartInput from "../components/UartInput.vue";
import UartLog from "../components/UartLog.vue";

const r3m0teStore = useR3m0teStore();
const { connected, remoteMsg } = storeToRefs(r3m0teStore);

const { height: screenH } = useWindowSize();

const logH = computed(() => {
  //TODO(buglloc): WTF?
  return screenH.value - 64 - 72 - 48;
});

const logEl = ref(null);
function sendMessage(msg: string) : boolean {
  const data = msg + "\n";
  const ok = r3m0teStore.sendMsg("uart.tx", {data: btoa(data)}, true);
  logEl.value?.consumeTx(data);
  return true;
};

watch(remoteMsg, (msg?: Msg) => {
  if (msg?.kind != "uart.rx") {
    return;
  }

  logEl.value?.consumeRx(atob((msg as MsgUartRx).data));
});

const states = {
  status: {
    icon: mdiConnection,
    name: "status",
    title: "Connected",
    isActive: connected,
    isImportant: true,
  },
};
</script>
