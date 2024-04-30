<template>
  <VList
    :data="rows"
    :style="{
      height: `${height}px`,
    }"
    ref="listEl"
    class="logs"
    reverse
    #default="item"
    @scroll="onScroll"
  >
    <span
      :key="item.id"
      class="text-grey-lighten-1 row-container"
    >
    <pre><Ansi class="row">{{ formatItem(item) }}</Ansi></pre>
    </span>
  </VList>
</template>

<script setup lang="ts">
import Ansi from "ansi-to-vue3";
import { VList } from "virtua/vue";
import { watch, ref } from "vue"
import { useDateFormat } from '@vueuse/core'

export interface Props {
  height?: number
  maxRows?: number
}

enum RowKind {
  Tx,
  Rx,
  Info,
  Warn,
}

interface Row {
  id: number
  when: Date
  value: string
  kind: RowKind
  shallow?: boolean
}

const shouldStickToBottom = ref(true);

const props = withDefaults(defineProps<Props>(), {
  height: 600,
  maxRows: 10<<10,
});

const rows = ref<Array<Row>>([{
  id: -1,
  when: new Date(),
  kind: RowKind.Info,
  value: "No data received yet...",
  shallow: true,
}]);

const listEl = ref(null);

defineExpose({
  consumeRx: pushRows.bind(null, RowKind.Rx),
  consumeTx: (rawData: string) => {
    pushRows(RowKind.Tx, rawData);
    shouldStickToBottom.value = true;
  }
});

watch(
  () => rows.value.length,
  (length: number) => {
    if (!shouldStickToBottom.value) {
      return;
    }

    listEl.value?.scrollToIndex(length - 1, {
      align: "start"
    });
  }
)

function onScroll(offset: number) {
  shouldStickToBottom.value = offset - listEl.value?.scrollSize + listEl.value?.viewportSize >= -32;
}

const rowId = ref(0);
function pushRows(kind: RowKind, rawData: string) {
  const newLines = rawData.split("\n");
  if (!newLines.length) {
    return;
  }

  let curRows = rows.value;
  const curRow = curRows.pop();
  if (!!curRow && !curRow.shallow) {
    const firstLine = newLines.shift();
    if (!!firstLine) {
      if (curRow.kind == kind) {
        curRow.value += firstLine;
        curRows.push(curRow);
      } else if (curRow.value.length) {
        curRows.push(curRow);
        newLines.unshift(firstLine);
      } else {
        newLines.unshift(firstLine);
      }
    }
  }

  const toRemove = curRows.length + newLines.length - props.maxRows;
  if (toRemove > 0) {
    curRows = curRows.splice(toRemove);
  }

  rows.value = curRows.concat(newLines.map((line: string) => ({
    id: rowId.value++,
    when: new Date(),
    kind: kind,
    value: line,
  })));
}

function formatItem(item: any) {
  // let color: string = "";
  // let color: string = "\x1B[0;37m";
  // let pad: string = "   ";
  // switch (item.kind) {
  //   case RowKind.Rx:
  //     color = "\x1B[0;97m";
  //     pad = " < ";
  //     break;

  //   case RowKind.Tx:
  //     color = "\x1B[0;93m";
  //     pad = " > ";
  //     break;

  //   case RowKind.Warn:
  //     color = "\x1B[0;91m";
  //     pad = " ! ";
  //     break;

  //   case RowKind.Info:
  //     color = "\x1B[0;95m";
  //     pad = " # ";
  //     break;
  // };

  const timeFormat = "HH:mm:ss";
  return `[${useDateFormat(item.when, timeFormat).value}] ${item.value}`;
  // return `[${useDateFormat(item.when, timeFormat).value}]${color}${pad}${item.value}\x1B[0m`;
}
</script>

<style scoped lang="scss">
// @import "@fontsource/roboto-mono/latin-400.css";

.logs {
  overflow-y: auto;
  line-height: 16px;
}

.row-container {
  height: 16px;
}

.row {
  font-family: 'Roboto Mono', monospace;
  font-size: 12px;
}
</style>
