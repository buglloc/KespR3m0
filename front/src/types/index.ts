
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

export interface BaseMsg {
  kind: string;
}

export interface MsgUartRx extends BaseMsg {
  data: string;
}

export type Msg = MsgUartRx;
