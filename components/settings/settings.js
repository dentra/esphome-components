// https://github.com/mujahidfa/preact-htm-signals-standalone
import {
  html,
  render,
  signal,
  Component,
  effect,
  computed,
} from "https://cdn.jsdelivr.net/npm/preact-htm-signals-standalone/dist/standalone.js"

// import { html, render, Component } from "https://esm.sh/htm/preact"
// import { signal, useSignal, computed, effect } from "https://esm.sh/@preact/signals"

function transformSections(source) {
  const defaults = {
    value: "",
    min: "",
    max: "",
    help: "",
  }

  const types = {
    1: "int",
    2: "int",
    3: "int",
    4: "int",
    5: "int",
    6: "int",
    7: "int",
    8: "int",
    9: "str",
    10: "bool",
    11: "float",
    12: "float",
    13: "password",
    14: "mac",
    15: "int",
    16: "timeout",
  }

  const sections = new Map()
  sections.set("", []) // empty section is always first
  for (const val of source) {
    const section = val.section || ""
    for (const def in defaults) {
      if (!(def in val)) {
        val[def] = defaults[def]
      }
    }
    val.type = types[val.type] || "str"
    if ((val.name || "").length == 0) {
      val.name = val.key
    }
    const help = val.help || ""
    if (help.length > 0 && !help.endsWith(".")) {
      val.help = help + "."
    }
    var current = sections.get(section)
    if (!current) {
      current = []
      sections.set(section, current)
    }
    val.id = val.key
    current.push(val)
  }

  return sections
}

const rfc_dialog = signal(false)
const changes = signal([])
const cchanges = computed(() => changes.value.length)

function FormFields(props) {
  function FormSection(props) {
    if (props.name.length > 0) {
      return html` <details>
        <summary>${props.name}</summary>
        ${props.fields}
      </details>`
    }
    return props.fields
  }

  class FormField extends Component {
    inputs = {
      str: (props, onChange) =>
        html`<input
          type="text"
          placeholder="${props.name}"
          minlength="${props.min}"
          maxlength="${props.max}"
          required=${props.min > 0}
          aria-invalid=${this.state.invalid}
          value=${this.state.value}
          onChange=${onChange}
        />`,
      password: (props, onChange) =>
        html`<input
          type="password"
          id="$${props.id}"
          placeholder="${props.name}"
          minlength="${props.min}"
          maxlength="${props.max}"
          aria-invalid=${this.state.invalid}
          value=${this.state.value}
          onChange=${onChange}
        />`,
      mac: (props, onChange) =>
        html`<input
          type="text"
          placeholder="${props.name}"
          minlength="17"
          maxlength="17"
          pattern="[da-fA-F]{2}:[da-fA-F]{2}:[da-fA-F]{2}:[da-fA-F]{2}:[da-fA-F]{2}:[da-fA-F]{2}"
          aria-invalid=${this.state.invalid}
          value=${this.state.value}
          onChange=${onChange}
        />`,
      bool: (props, onChange) =>
        html`<input
          type="checkbox"
          placeholder="${props.name}"
          role="switch"
          checked=${this.state.value}
          onChange=${onChange}
        />`,
      int: (props, onChange) =>
        html`<input
          type="number"
          placeholder="${props.name}"
          min="${props.min}"
          max="${props.max}"
          step="1"
          aria-invalid=${this.state.invalid}
          required
          value=${this.state.value}
          onChange=${onChange}
        />`,
      timeout: (props, onChange) =>
        html`<input
          type="number"
          placeholder="${props.name}"
          min="0"
          max="${props.max}"
          step="1"
          aria-invalid=${this.state.invalid}
          required
          value=${this.state.value}
          onChange=${onChange}
        />`,
      float: (props, onChange) =>
        html`<input
          type="number"
          placeholder="${props.name}"
          min="${props.min}"
          max="${props.max}"
          step="${props.accuracy_decimals > 0 ? Math.pow(10.0, -props.accuracy_decimals) : 0.1}"
          aria-invalid=${this.state.invalid}
          required
          value=${this.state.value}
          onChange=${onChange}
        />`,
    }

    constructor(props) {
      super(props)
      this.state = { value: props.value, name: "" }
    }

    onChange(e) {
      const target = e.target
      const valid = target.validity.valid
      const value = target.type == "checkbox" ? target.checked : target.value
      const id = this.props.id
      const name = valid && this.props.value != value ? id : ""
      const new_changes = changes.value.filter((v) => v != id)
      if (name) {
        new_changes.push(id)
      }
      changes.value = new_changes
      this.setState({ value: value, name: name, invalid: !valid })
    }

    render(props) {
      return html`<label>
        ${props.type != "bool" && props.name} ${this.inputs[props.type](props, (e) => this.onChange(e))}
        ${props.type == "bool" && props.name}
        <small>${props.help}${props.type == "timeout" && " Can be disabled by setting this to 0."}</small>
        <input type="hidden" name=${this.state.name} value=${this.state.value} />
      </label>`
    }
  }

  const res = []
  for (const [section, vars] of props.sections) {
    if (vars.length > 0) {
      const fields = vars.map((item) => html`<${FormField} ...${item} />`)
      res.push(html`<${FormSection} name="${section}" fields="${fields}" />`)
    }
  }
  return res
}

class FactoryResetConfirm extends Component {
  form = null
  setForm = (form) => (this.form = form)

  onChange = (e) => {
    this.setState({ checked: !this.state.checked })
  }

  onCancel = (e) => {
    e.preventDefault()
    rfc_dialog.value = false
    this.setState({ checked: false })
  }

  onSubmit = (e) => {
    if (this.form) {
      this.form.submit()
      rfc_dialog.value = false
      this.setState({ checked: false })
    }
  }

  render(_, { checked }) {
    return html`
      <form action="reset" method="post" ref=${this.setForm}>
        <dialog open=${rfc_dialog.value}>
          <article>
            <header>Factory Reset confirmation</header>
            <p>
              CAUTION: All credentials, global variables, counters and saved states stored in non-volatile memory will
              be lost with no chance of recovering them.
            </p>
            <p>
              This will also reset the Wi-Fi settings, which will make the device offline. You'll need to be in close
              proximity to your device to configure it again using a built-in WiFi access point and captive portal.
            </p>
            <p>
              <input type="checkbox" name="confirm" checked=${checked} onChange=${this.onChange} />
              Yes, I confirm that I am aware of the effect.
            </p>
            <footer>
              <button onClick=${this.onCancel}>Cancel</button>
              <button onClick=${this.onSubmit} disabled=${!checked} class="red">Confirm</button>
            </footer>
          </article>
        </dialog>
      </form>
    `
  }
}

class AppForm extends Component {
  form = null

  constructor(props) {
    super(props)
  }

  setForm = (form) => (this.form = form)

  onSubmit = (e) => {
    e.preventDefault()
    if (this.form && this.form.checkValidity()) {
      this.form.submit()
    }
  }

  onMainMenu = (e) => {
    e.preventDefault()
    window.location.href = this.props.menu
  }

  onFactoryReset = (e) => {
    e.preventDefault()
    rfc_dialog.value = true
  }

  shouldComponentUpdate = (nextProps, nextState) => {
    return false
  }

  submitButton = (props) => {
    return html`<input
      type="submit"
      value="Save and Reboot"
      disabled=${changes.value.length == 0 || !this.form || !this.form.checkValidity()}
    />`
  }

  render(props) {
    return html`<form ref=${this.setForm} action="" method="post" novalidate onSubmit=${this.onSubmit}>
      <article>
        <${FormFields} sections=${props.sections} />
        <footer>
          <${this.submitButton} />
          <input type="button" onClick=${this.onFactoryReset} value="Factory Reset" />
          ${props.menu && html`<input type="button" onClick=${this.onMainMenu} value="Main Menu" />`}
        </footer>
      </article>
    </form>`
  }
}

function App(props) {
  return html`<main>
    <h1>${props.title}</h1>
    <p>
      ${props.project_name && html`${props.project_name.replace(".", " ")}: ${props.project_version},`} ESPHome:
      ${props.esphome_version}, ${props.compilation_time}
    </p>
    <${AppForm} sections=${props.sections} menu=${props.menu} />
    <${FactoryResetConfirm} />
  </main>`
}

const data = await (await fetch("settings.json")).json()
document.title = data.t + " settings"
render(
  html`<${App}
    title=${document.title}
    sections=${transformSections(data.v)}
    menu=${data.m}
    project_name=${data.pn}
    project_version=${data.pv}
    esphome_version=${data.ev}
    compilation_time=${data.et}
  />`,
  document.body
)
