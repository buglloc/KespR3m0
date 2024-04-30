
export interface BaseApp {
  disabled: boolean;
  started: boolean;
}

export interface AppUsbKb extends BaseApp {
}

export interface AppUart extends BaseApp {
  config: {
    braud: number;
  };
}

export type App = AppUsbKb | AppUsbKb;

export interface BaseEvent {
  cmd: string;
}

export interface EventUartRx extends BaseEvent {
  data: string;
}

export type Event = EventUartRx;
