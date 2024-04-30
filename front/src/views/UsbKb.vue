<template>
  <StatusBar
    :icons="[states.kbr, states.status]"
    title="USB Keyboard"
  />

  <v-container class="fill-height" fluid>
    <v-responsive
      class="align-center fill-height mx-auto"
      max-width="1024"
    >
      <div v-if="!isMobile">
        <v-img
          class="mb-4"
          src="@/assets/eyes.webp"
        />
      </div>
      <v-footer app>
        <VirtualKeyboard />
      </v-footer>
    </v-responsive>
  </v-container>
</template>

<script setup lang="ts">
import { computed } from "vue";
import { useDisplay } from "vuetify";
import { storeToRefs } from "pinia";
import { mdiConnection, mdiKeyboard } from "@mdi/js";
import { useR3m0teStore } from "../stores/remote";
import StatusBar from "../components/StatusBar.vue";
import { reactive } from "vue";

const r3m0teStore = useR3m0teStore();
const { connected, remoteEvent } = storeToRefs(r3m0teStore);

const { mobile:isMobile } = useDisplay();

const states = reactive({
  status: {
    icon: mdiConnection,
    name: "status",
    title: "Connected",
    isActive: connected,
    isImportant: true,
  },
  kbr: {
    icon: mdiKeyboard,
    name: "keyboard",
    title: "Keyboard",
    isActive: computed(() => false),
  },
});
</script>
