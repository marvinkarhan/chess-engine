import { Directive, ElementRef, Input } from '@angular/core';
import { fromEvent } from 'rxjs';
import { filter, switchMap, take, takeUntil, tap } from 'rxjs/operators';

@Directive({
  selector: '[draggable]',
})
export class DraggableDirective {
  private _draggableBounds!: number;
  @Input() set draggableBounds(draggableBounds: number) {
    this.elDim = draggableBounds / 8;
    this._draggableBounds = draggableBounds;
  }
  get draggableBounds() {
    return this._draggableBounds;
  }

  elDim: number = 0;
  initPos!: { top: number; left: number; x: number; y: number };
  pos = { top: 0, left: 0, x: 0, y: 0 };

  constructor(elementRef: ElementRef) {
    this.elDim = this.draggableBounds / 8;
    // Drag and drop for navigation
    const element = elementRef.nativeElement as HTMLElement;
    element.style.cursor = 'grab';
    fromEvent<MouseEvent>(element, 'mousedown')
      .pipe(
        filter((event) => event.target === element),
        tap((event) => {
          this.centerElFromEventOnCursor(element, event);
          element.style.cursor = 'grabbing';
          element.style.zIndex = '1000';
          const matrix = this.getMatrix(element);
          // set current posistins
          this.pos = {
            // The current scroll
            left: +matrix[4],
            top: +matrix[5],
            // Get the current mouse position
            x: event.clientX,
            y: event.clientY,
          };
          this.initPos = Object.assign({}, this.pos);
          // listen for mouse movement
          fromEvent<MouseEvent>(document, 'mousemove')
            .pipe(
              // terminate when mouse is let go
              takeUntil(
                fromEvent<MouseEvent>(document, 'mouseup').pipe(take(1))
              ),
              tap((event) => {
                // How far the mouse has been moved
                const dx = event.clientX - this.pos.x;
                const dy = event.clientY - this.pos.y;

                // Scroll the element
                this.setTranslate(
                  element,
                  this.pos.left + dx,
                  this.pos.top + dy
                );
              })
            )
            .subscribe();
        }),
        switchMap(() =>
          // wait until mouse is releases
          fromEvent<MouseEvent>(document, 'mouseup').pipe(take(1))
        ),
        tap(() => {
          this.resetStyle(element);
        })
      )
      .subscribe();
  }

  private resetStyle(element: HTMLElement) {
    element.style.cursor = 'grab';
    element.style.zIndex = '';
    this.checkBounds(element);
    this.centerOnCell(element);
  }

  private checkBounds(element: HTMLElement) {
    const matrix = this.getMatrix(element);
    if (
      +matrix[4] > this.draggableBounds - this.elDim / 2 ||
      +matrix[5] > this.draggableBounds - this.elDim / 2 ||
      +matrix[4] < 0 - this.elDim / 2 ||
      +matrix[5] < 0 - this.elDim / 2
    ) {
      this.setTranslate(element, this.initPos?.left, this.initPos?.top);
    }
  }

  private centerOnCell(element: HTMLElement) {
    const matrix = this.getMatrix(element);
    this.setTranslate(
      element,
      (((+matrix[4] + this.elDim / 2) / this.elDim) | 0) * this.elDim,
      (((+matrix[5] + this.elDim / 2) / this.elDim) | 0) * this.elDim
    );
  }

  private centerElFromEventOnCursor(element: HTMLElement, event: MouseEvent) {
    let xDiff = event.offsetX - this.elDim / 2;
    let yDiff = event.offsetY - this.elDim / 2;
    const matrix = this.getMatrix(element);
    this.setTranslate(element, +matrix[4] + xDiff, +matrix[5] + yDiff);
  }

  getMatrix(element: HTMLElement) {
    const style = window.getComputedStyle(element);
    return (style.transform.match(/matrix.*\((.+)\)/) || [])[1]?.split(', ');
  }

  setTranslate(element: HTMLElement, x: number, y: number) {
    element.style.transform = `translate(${x}px, ${y}px)`;
  }
}
