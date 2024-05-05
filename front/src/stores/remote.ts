import { watch, computed, ref, reactive } from "vue";
import { defineStore } from "pinia";
import { useWebSocket } from "@vueuse/core"
import type { App, AppUsbKb, AppUart, Msg } from "../types";
import { v4 as uuidv7 } from "uuid";

function buildMsg(kind: string, data?: any): string {
  return JSON.stringify({
    kind,
    id: uuidv7(),
    ...data,
  });
}

export const useR3m0teStore = defineStore('r3m0te', () => {
  const wsProto = window.location.protocol.replace('http', 'ws');
  const wsUrl = `${wsProto}//${window.location.host}/api/ws`;
  const {
    status:wsStatus,
    data:wsData,
    send:wsSend,
  } = useWebSocket(wsUrl, {
    heartbeat: {
      message: buildMsg("ping"),
      interval: 5000,
      pongTimeout: 1000,
    },
    autoReconnect: true,
  });

  const initialized = ref(false);
  const remoteMsg = ref<Msg>();
  const appUsbKb: AppUsbKb = reactive({
    disabled: true,
    started: false,
  });
  const appUart: AppUart = reactive({
    disabled: true,
    started: false,
    config: reactive({
      braud: 9600,
    }),
  });

  function sendMsg(kind: string, params?: any, buffered?: boolean): boolean {
    return wsSend(buildMsg(kind, params), buffered == true);
  }

  watch(wsStatus, (status) => {
    if (status != "OPEN") {
      return;
    }

    sendMsg("state");
  });

  function appByName(name: string): App {
    switch (name) {
    case "usbkb":
      return appUsbKb;
    case "uart":
      return appUart;
    default:
      throw Error(`unexpected app: ${name}`);
    }
  }

  watch(wsData, (rawMsg: string) => {
    if (!rawMsg) {
      return;
    }

    try {
      const msg = JSON.parse(rawMsg);
      switch (msg.kind) {
      case "state": {
        initialized.value = true;
        const apps = msg.apps as Record<string, App>;
        for (const name in apps) {
          const app = appByName(name);
          app.disabled = false;
          app.started = apps[name].started;
        }
        break;
      }

      default:
        remoteMsg.value = msg;
      }
    } catch (error) {
      console.log(`unable to parse ws data ('${error}'): ${wsData.value}`);
    }
  });

  return {
    connected: computed(() => wsStatus.value == "OPEN"),
    connecting: computed(() => wsStatus.value == "CONNECTING"),
    remoteMsg,
    initialized,
    appUsbKb,
    appUart,
    appByName,
    sendMsg,
    switchApp: (name: string): boolean => {
      return sendMsg("app.switch", {"app": name});
    }
  };
});
