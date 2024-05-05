<template>
    <v-row
      align="center"
      class="h-100"
    >
      <v-spacer />
      <v-col
        v-for="[key, app] in Object.entries(apps)"
        :key="key"
        cols="2"
        md="2"
        align="center"
        class="pa-1"
      >
        <v-btn
          :prepend-icon="app.icon"
          :disabled="app.disabled.value"
          :loading="app.loading.value"
          @click="startApp(app)"
          class="pa-4"
          stacked
          variant="tonal"
          size="large"
        >
          {{ app.title }}
        </v-btn>
      </v-col>
      <v-spacer />
    </v-row>
</template>
<script setup lang="ts">
import { ref, Ref, computed } from "vue"
import { useRouter } from "vue-router"
import { storeToRefs } from "pinia";
import { useR3m0teStore } from "../stores/remote";
import {
  mdiKeyboardOutline,
  mdiCardTextOutline,
} from "@mdi/js";

const r3m0teStore = useR3m0teStore();
const { appUsbKb, appUart } = storeToRefs(r3m0teStore);

interface App {
  name: string
  icon: string
  loading: Ref
  disabled: Ref
  title: string
}

interface Apps {
  usbkb?: App
  uart?: App
}

const apps: Apps = {
  usbkb: {
    name: "usbkb",
    icon: mdiKeyboardOutline,
    title: "USB Keyboard",
    loading: ref(false),
    disabled: computed(() => appUsbKb.value.disabled),
  },
  uart: {
    name: "uart",
    icon: mdiCardTextOutline,
    title: "UART Logger",
    loading: ref(false),
    disabled: computed(() => appUart.value.disabled),
  },
};

const router = useRouter();
async function startApp(app: App) {
  if (!r3m0teStore.appByName(app.name).started) {
    app.loading.value = true;
    const ok = r3m0teStore.startApp(app.name);
    app.loading.value = false;
    if (!ok) {
      alert("ship happens");
    }
  }

  router.push(`/${app.name}`);
}
</script>
