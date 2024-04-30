<template>
  <v-app-bar class="statuses">

  <template v-slot:prepend>
    <v-app-bar-nav-icon></v-app-bar-nav-icon>
  </template>

  <v-app-bar-title>
    <RouterLink to="/">KespR3m0</RouterLink> | {{ title }}
  </v-app-bar-title>

  <template v-slot:append>
    <template v-for="icon in icons" :key="icon.name">
      <v-icon
        :icon="icon.icon"
        :class="{
          'active': icon.isActive,
          'important': icon.isImportant,
          'mx-2': true,
        }"
        :title="icon.isActive"
      />
    </template>
  </template>

  </v-app-bar>
  <!-- <v-navigation-drawer

    temporary
  >
  </v-navigation-drawer> -->
</template>

<script setup lang="ts">
interface Icon {
  icon: string
  name: string
  title: string
  isActive: boolean
  isImportant?: boolean
}

export interface Props {
  title: string
  icons: Icon[]
}

defineProps<Props>();
</script>

<style scoped>
.statuses  .v-icon.important:not(.active) {
  color: rgb(228, 6, 6);
}

.statuses .v-icon.active {
  color: rgb(114, 192, 253) !important;
}
</style>
