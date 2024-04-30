<template>
  <v-text-field
    v-model="message"
    :append-icon="mdiSend"
    :clear-icon="mdiCloseCircle"
    :disabled="sending"
    label="Message"
    type="text"
    variant="filled"
    clearable
    hide-details
    @keydown.enter.prevent="sendMessage"
    @click:append="sendMessage"
    @click:clear="clearMessage"
  />
</template>

<script setup lang="ts">
import { ref } from "vue"
import { mdiSend, mdiCloseCircle } from "@mdi/js";

export interface Props {
  onMessage: (msg: string) => boolean
}

const props = defineProps<Props>();

const message = ref("");
const sending = ref(false);

function sendMessage () {
  sending.value = true;
  if (props.onMessage(message.value)) {
    clearMessage();
  }
  sending.value = false;
};

function clearMessage () {
  message.value = "";
};
</script>
