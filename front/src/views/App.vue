<template>
  <Message v-if="loading">
    Loading, please be patient...
  </Message>
  <Message v-else-if="app.disabled">
    Sorry, but application <strong class="text-red-lighten-1">{{ title }}</strong> is disabled :(
  </Message>
  <slot v-else-if="app.started"></slot>
  <Message v-else>
    Sorry, but application <strong class="text-red-lighten-1">{{ title }}</strong> is not started yet...
  </Message>
</template>

<script setup lang="ts">
import { computed } from "vue";
import { storeToRefs } from "pinia";
import { watchOnce } from "@vueuse/core";
import Message from "../views/Message.vue";
import { useR3m0teStore } from "../stores/remote";


export interface Props {
  name: string
  title: string
}

const props = defineProps<Props>();

const r3m0teStore = useR3m0teStore();
const { initialized } = storeToRefs(r3m0teStore);
const loading = computed(() => !initialized.value);

const app = r3m0teStore.appByName(props.name);

watchOnce(initialized, () => {
  if (app.disabled || app.started) {
    return;
  }

  r3m0teStore.startApp(props.name);
});
</script>
